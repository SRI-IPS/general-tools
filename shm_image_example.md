# Transporting Images with Shared Memory (SHM)

The `dispatch` library's SHM transport is designed for large data structures like camera frames, where copying bytes through the standard network stack (ZMQ) would be prohibitively slow. 

Instead of sending the image data itself over ZMQ, the publisher writes it to a shared memory segment and sends a small `SharedMemoryHandle` message. The subscriber then maps that same memory segment to read the data directly.

> [!IMPORTANT]
> **Local-Only constraint:** SHM only works between processes on the **same physical machine**. It will not fall back to network transport if the receiver is on a different host.

---

## C++ Example: Publishing and Subscribing to Images

### 1. The Publisher (Image Producer)

The publisher pre-allocates a shared memory region and writes the image data directly into it.

```cpp
#include "node.h"
#include "shm_publisher.h"
#include <opencv2/opencv.hpp> // Assuming OpenCV for image handling

using namespace a17::dispatch;

int main() {
    Node node("image_producer");
    auto pub = node.createPublisher("camera/raw");

    // 1. Pre-allocate SHM for a 640x480 RGB image
    const uint32_t imageSize = 640 * 480 * 3;
    ShmPublisher shmPub(imageSize);

    while (node.ok()) {
        cv::Mat frame = captureFrame(); // Your image source

        // 2. Copy image data directly into SHM
        // In a real zero-copy scenario, you might tell the camera driver 
        // to write directly to shmPub.ptr().
        std::memcpy(shmPub.ptr(), frame.data, imageSize);

        // 3. Publish the SHM handle over the ZMQ publisher
        shmPub.publish(*pub);

        std::this_thread::sleep_for(std::chrono::milliseconds(33)); // 30 FPS
    }
    return 0;
}
```

### 2. The Subscriber (Image Consumer)

The subscriber listens for messages on the topic and uses `ShmSubscriber` to map and read the memory.

```cpp
#include "node.h"
#include "shm_subscriber.h"
#include <opencv2/opencv.hpp>

using namespace a17::dispatch;

int main() {
    Node node("image_consumer");
    ShmSubscriber shmSub;

    auto sub = node.createSubscriber("camera/raw", [&](azmq::message_vector& msg) {
        // 1. Use ShmSubscriber to decode the handle and map the memory
        shmSub.receive(msg, [&](const void* ptr, uint32_t size) {
            // 2. 'ptr' is a direct pointer to the memory shared by the publisher.
            // Wrap it in a cv::Mat without copying.
            cv::Mat frame(480, 640, CV_8UC3, const_cast<void*>(ptr));
            
            // 3. Process the frame
            cv::imshow("SHM Image", frame);
            cv::waitKey(1);
            
            // NOTE: 'ptr' is ONLY valid inside this lambda. 
            // The memory is unmapped once the lambda returns.
        });
    });

    node.spin();
    return 0;
}
```

---

## Python Example: Mixed-Language workflows

SHM is fully compatible between C++ and Python. Here is how you would consume the C++ image stream in Python.

### The Subscriber (Python)

```python
import cv2
import numpy as np
from a17.dispatch.dispatch import Node
from a17.dispatch.dispatch_shm import ShmSubscriber

def main():
    node = Node("py_image_consumer")
    shm_sub = ShmSubscriber()

    def on_image(arr):
        # 'arr' is a zero-copy numpy view of the shared memory.
        # It is only valid during this callback.
        cv2.imshow("Python SHM View", arr)
        cv2.waitKey(1)

    # Subscribe to the topic
    sub = node.create_subscriber("camera/raw")
    
    while True:
        # Receive the multipart ZMQ message
        frames = sub.recv_multipart()
        
        # Map the SHM segment based on the handle in the message
        # Specify the expected shape and dtype for zero-copy numpy conversion
        shm_sub.receive(frames, callback=on_image, 
                         dtype=np.uint8, shape=(480, 640, 3))

if __name__ == "__main__":
    main()
```

---

## Key Takeaways for Image Transport

1.  **Lifetime:** The memory is owned by the `ShmPublisher`. If the publisher is destroyed, the memory is unlinked (though existing mappings remain valid until unmapped).
2.  **Concurrency:** The publisher can update the memory and call `publish()` again. Subscribers map the segment, read it, and unmap it immediately.
3.  **Efficiency:** For a 1080p RGB image (~6MB), SHM avoids copying 6MB per subscriber, which significantly reduces CPU usage and latency compared to standard ZMQ transport.
