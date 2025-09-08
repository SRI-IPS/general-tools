load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# ------------------------------------------------------------------------------
# NOTE: This MUST be kept exactly in-sync with the protobuf in //CMakeLists.txt.
http_archive(
    name = "com_google_protobuf",
    sha256 = "390191a0d7884b3e52bb812c440ad1497b9d484241f37bb8e2ccc8c2b72d6c36",
    strip_prefix = "protobuf-3.19.3",
    urls = ["https://github.com/google/protobuf/archive/v3.19.3.tar.gz"],
)

http_archive(
    name = "com_google_protobuf_cc",
    sha256 = "390191a0d7884b3e52bb812c440ad1497b9d484241f37bb8e2ccc8c2b72d6c36",
    strip_prefix = "protobuf-3.19.3",
    urls = ["https://github.com/google/protobuf/archive/v3.19.3.tar.gz"],
)

# ------------------------------------------------------------------------------
## rules_python - December 6, 2022 (must be before call to rules_proto_dependencies())
http_archive(
    name = "rules_python",
    sha256 = "497ca47374f48c8b067d786b512ac10a276211810f4a580178ee9b9ad139323a",
    strip_prefix = "rules_python-0.16.1",
    url = "https://github.com/bazelbuild/rules_python/archive/refs/tags/0.16.1.tar.gz",
)

# rules_proto - December 27, 2022
http_archive(
    name = "rules_proto",
    sha256 = "dc3fb206a2cb3441b485eb1e423165b231235a1ea9b031b4433cf7bc1fa460dd",
    strip_prefix = "rules_proto-5.3.0-21.7",
    urls = [
        "https://github.com/bazelbuild/rules_proto/archive/refs/tags/5.3.0-21.7.tar.gz",
    ],
)

load("@rules_proto//proto:repositories.bzl", "rules_proto_dependencies", "rules_proto_toolchains")

rules_proto_dependencies()

rules_proto_toolchains()

## boringssl - January 9, 2023 (must be before http_archive io_bazel_rules_go and org_golang_google_grpc)
# https://github.com/google/boringssl/blob/master/INCORPORATING.md
git_repository(
    name = "boringssl",
    commit = "f8a10eeef9549601a91c4525f55bf8a59d338eff",
    remote = "https://boringssl.googlesource.com/boringssl",
)

# Rules Go - June 28, 2023
http_archive(
    name = "io_bazel_rules_go",
    sha256 = "51dc53293afe317d2696d4d6433a4c33feedb7748a9e352072e2ec3c0dafd2c6",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_go/releases/download/v0.40.1/rules_go-v0.40.1.zip",
        "https://github.com/bazelbuild/rules_go/releases/download/v0.40.1/rules_go-v0.40.1.zip",
    ],
)

load("@io_bazel_rules_go//go:deps.bzl", "go_register_toolchains", "go_rules_dependencies")

go_rules_dependencies()

go_register_toolchains(version = "1.20.5")

# Gazelle : depends on Rules Go
http_archive(
    name = "bazel_gazelle",
    sha256 = "b8b6d75de6e4bf7c41b7737b183523085f56283f6db929b86c5e7e1f09cf59c9",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-gazelle/releases/download/v0.31.1/bazel-gazelle-v0.31.1.tar.gz",
        "https://github.com/bazelbuild/bazel-gazelle/releases/download/v0.31.1/bazel-gazelle-v0.31.1.tar.gz",
    ],
)

load("@bazel_gazelle//:deps.bzl", "gazelle_dependencies", "go_repository")

gazelle_dependencies()

# deps for rules_go
go_repository(
    name = "org_golang_google_grpc",
    build_file_proto_mode = "disable",
    importpath = "google.golang.org/grpc",
    sum = "h1:fPVVDxY9w++VjTZsYvXWqEf9Rqar/e+9zYfxKK+W+YU=",
    version = "v1.50.0",
)

go_repository(
    name = "org_golang_google_protobuf",
    importpath = "google.golang.org/protobuf",
    sum = "h1:xsAVV57WRhGj6kEIi8ReJzQlHHqcBYCElAvkovg3B/4=",
    version = "v1.28.1",
)

go_repository(
    name = "com_github_google_go_cmp",
    importpath = "github.com/google/go-cmp",
    sum = "h1:xsAVV57WRhGj6kEIi8ReJzQlHHqcBYCElAvkovg3B/4=",
    version = "v0.4.0",
)

go_repository(
    name = "org_golang_x_net",
    importpath = "golang.org/x/net",
    sum = "h1:aWJ/m6xSmxWBx+V0XRHTlrYrPG56jKsLdTFmsSsCzOM=",
    version = "v0.9.0",
)

go_repository(
    name = "org_golang_x_text",
    importpath = "golang.org/x/text",
    sum = "h1:2sjJmO8cDvYveuX97RDLsxlyUxLl+GHoLxBiRdHllBE=",
    version = "v0.9.0",  # April 04, 2023
)

go_repository(
    name = "org_golang_x_sys_unix",
    importpath = "golang.org/x/sys",
    sum = "h1:3jlCCIQZPdOYu1h8BkNvLz8Kgwtae2cagcG/VamtZRU=",
    version = "v0.7.0",
)

go_repository(
    name = "org_golang_x_crypto",
    importpath = "golang.org/x/crypto",
    sum = "h1:wBqGXzWJW6m1XrIKlAH0Hs1JJ7+9KBwnIO8v66Q9cHc=",
    version = "v0.14.0",
)

# rules_docker - June 22, 2022
http_archive(
    name = "io_bazel_rules_docker",
    sha256 = "b1e80761a8a8243d03ebca8845e9cc1ba6c82ce7c5179ce2b295cd36f7e394bf",
    urls = ["https://github.com/bazelbuild/rules_docker/releases/download/v0.25.0/rules_docker-v0.25.0.tar.gz"],
)

load("@io_bazel_rules_docker//repositories:repositories.bzl", container_repositories = "repositories")

container_repositories()

load("@io_bazel_rules_docker//container:pull.bzl", "container_pull")
load("@io_bazel_rules_docker//contrib:dockerfile_build.bzl", "dockerfile_image")

container_pull(
    name = "ubuntu2004",
    digest = "sha256:b795f8e0caaaacad9859a9a38fe1c78154f8301fdaf0872eaf1520d66d9c0b98",
    registry = "docker.io",
    repository = "ubuntu",
    tag = "20.04",
)

dockerfile_image(
    name = "service_dockerfile",
    dockerfile = "//a17/backend/docker/service:Dockerfile",
)

load("@io_bazel_rules_docker//go:image.bzl", _go_image_repos = "repositories")

_go_image_repos()

# ------------------------------------------------------------------------------
# rules_proto_grpc - December 04, 2022
http_archive(
    name = "rules_proto_grpc",
    sha256 = "fb7fc7a3c19a92b2f15ed7c4ffb2983e956625c1436f57a3430b897ba9864059",
    strip_prefix = "rules_proto_grpc-4.3.0",
    urls = ["https://github.com/rules-proto-grpc/rules_proto_grpc/archive/4.3.0.tar.gz"],
)

load("@rules_proto_grpc//:repositories.bzl", "rules_proto_grpc_repos", "rules_proto_grpc_toolchains")

rules_proto_grpc_toolchains()

rules_proto_grpc_repos()

# grpc gateway
load("@rules_proto_grpc//grpc-gateway:repositories.bzl", rules_proto_grpc_gateway_repos = "gateway_repos")

rules_proto_grpc_gateway_repos()

load("@grpc_ecosystem_grpc_gateway//:repositories.bzl", "go_repositories")

go_repositories()

# python grpc rules
load("@rules_proto_grpc//python:repositories.bzl", rules_proto_grpc_python_repos = "python_repos")

rules_proto_grpc_python_repos()

# python rules
load("@rules_python//python:repositories.bzl", "python_register_toolchains")

python_register_toolchains(
    name = "python3_9",
    # Available versions are listed in @rules_python//python:versions.bzl.
    # We recommend using the same version your team is already standardized on.
    python_version = "3.9",
)

load("@python3_9//:defs.bzl", "interpreter")
load("@rules_python//python:pip.bzl", "pip_parse")

pip_parse(
    python_interpreter_target = interpreter,
)

# cpp rules
load("@rules_proto_grpc//cpp:repositories.bzl", rules_proto_grpc_cpp_repos = "cpp_repos")

rules_proto_grpc_cpp_repos()

# grpc deps - not sure if needed
load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

grpc_deps()

# ------------------------------------------------------------------------------
# NOTE: This MUST be kept exactly in-sync with the gflags in //CMakeLists.txt.
http_archive(
    name = "com_github_gflags_gflags",
    sha256 = "ae27cdbcd6a2f935baa78e4f21f675649271634c092b1be01469440495609d0e",
    strip_prefix = "gflags-2.2.1",
    urls = ["https://github.com/gflags/gflags/archive/v2.2.1.tar.gz"],
)

bind(
    name = "gflags",
    actual = "@com_github_gflags_gflags//:gflags",
)

bind(
    name = "gflags_nothreads",
    actual = "@com_github_gflags_gflags//:gflags_nothreads",
)

# ------------------------------------------------------------------------------
# NOTE: This MUST be kept exactly in-sync with the glog in //CMakeLists.txt.
http_archive(
    name = "com_github_google_glog",
    build_file = "//third_party:glog.BUILD",
    sha256 = "7580e408a2c0b5a89ca214739978ce6ff480b5e7d8d7698a2aa92fadc484d1e0",
    strip_prefix = "glog-0.3.5",
    url = "https://github.com/google/glog/archive/v0.3.5.tar.gz",
)

bind(
    name = "glog",
    actual = "@com_github_google_glog//:glog",
)

# ------------------------------------------------------------------------------
# Needed by //a17/backend/...
go_repository(
    name = "com_github_stretchr_testify",
    commit = "0ab3ce1249292a7221058b9e370472bca8f04813",  # Jan 3, 2023
    importpath = "github.com/stretchr/testify",
)

go_repository(
    name = "com_github_stretchr_objx",
    importpath = "github.com/stretchr/objx",
    tag = "v0.5.0",  # Oct 14, 2022
)

go_repository(
    name = "com_github_davecgh_go_spew",
    importpath = "github.com/davecgh/go-spew",
    tag = "v1.1.1",  # Aug 17, 2018
)

# Needed by //a17/backend/cmd/clients/...
go_repository(
    name = "com_github_docopt_docopt_go",
    commit = "ee0de3bc6815ee19d4a46c7eb90f829db0e014b1",  # Jan 11, 2018
    importpath = "github.com/docopt/docopt-go",
)

# Needed by //a17/backend/services/...
go_repository(
    name = "com_github_google_uuid",
    importpath = "github.com/google/uuid",
    tag = "v1.3.0",  # July 12, 2021
)

go_repository(
    name = "com_github_globalsign_mgo",
    importpath = "github.com/globalsign/mgo",
    tag = "r2018.04.03",  # April 3, 2018
)

# Needed by //a17/backend/config
go_repository(
    name = "com_github_spf13_viper",
    importpath = "github.com/spf13/viper",
    tag = "v1.14.0",  # November 06, 2022
)

go_repository(
    name = "com_github_spf13_afero",
    importpath = "github.com/spf13/afero",
    tag = "v1.9.3",  # November 14, 2022
)

go_repository(
    name = "com_github_spf13_cast",
    importpath = "github.com/spf13/cast",
    tag = "v1.5.0",  # May 11, 2022
)

go_repository(
    name = "com_github_spf13_jwalterweatherman",
    importpath = "github.com/spf13/jwalterweatherman",
    tag = "v1.1.0",  # March 02, 2019
)

go_repository(
    name = "com_github_spf13_pflag",
    importpath = "github.com/spf13/pflag",
    tag = "v1.0.5",  # September 18, 2019
)

go_repository(
    name = "com_github_hashicorp_hcl",
    importpath = "github.com/hashicorp/hcl",
    tag = "v1.0.0",  # unknown
)

go_repository(
    name = "com_github_subosito_gotenv",
    importpath = "github.com/subosito/gotenv",
    tag = "v1.4.1",  # August 23, 2022
)

go_repository(
    name = "in_gopkg_ini_v1",
    importpath = "gopkg.in/ini.v1",
    tag = "v1.67.0",  # August 08, 2022
)

go_repository(
    name = "com_github_magiconair_properties",
    importpath = "github.com/magiconair/properties",
    tag = "v1.8.7",  # December 08, 2022
)

go_repository(
    name = "com_github_mitchellh_mapstructure",
    importpath = "github.com/mitchellh/mapstructure",
    tag = "v1.5.0",  # April 20, 2022
)

go_repository(
    name = "com_github_pelletier_go_toml",
    importpath = "github.com/pelletier/go-toml",
    strip_prefix = "go-toml-1.9.5",
    urls = ["https://github.com/pelletier/go-toml/archive/refs/tags/v1.9.5.tar.gz"],  # April 21, 2022
)

go_repository(
    name = "com_github_pelletier_go_toml_v2",
    importpath = "github.com/pelletier/go-toml/v2",
    strip_prefix = "go-toml-2.0.6",
    urls = ["https://github.com/pelletier/go-toml/archive/refs/tags/v2.0.6.tar.gz"],  # November 16, 2022
)

go_repository(
    name = "com_github_pmezard_go_difflib",
    importpath = "github.com/pmezard/go-difflib",
    tag = "v1.0.0",  # Aug 8, 2016
)

go_repository(
    name = "in_gopkg_yaml_v2",
    importpath = "gopkg.in/yaml.v2",
    tag = "v2.4.0",  # November 17, 2020
)

go_repository(
    name = "in_gopkg_yaml_v3",
    importpath = "gopkg.in/yaml.v3",
    tag = "v3.0.1",  # May 27, 2022
)

go_repository(
    name = "com_github_fsnotify_fsnotify",
    importpath = "github.com/fsnotify/fsnotify",
    tag = "v1.6.0",  # October 12, 2022
)

# Needed by //a17/backend/common/auth0
go_repository(
    name = "com_github_mendsley_gojwk",
    commit = "4d5ec6e58103388d6cb0d7d72bc72649be4f0504",  # December 17, 2014
    importpath = "github.com/mendsley/gojwk",
)

# Needed by //a17/backend/common/kubemq
go_repository(
    name = "com_github_kubemq_io_kubemq_go",
    commit = "4b4e96da5642194ba6ebba0dd3acc70f92240788",  # August 21, 2021
    importpath = "github.com/kubemq-io/kubemq-go",
)

go_repository(
    name = "com_github_gorilla_websocket",
    commit = "b65e62901fc1c0d968042419e74789f6af455eb9",  # March 19, 2020
    importpath = "github.com/gorilla/websocket",
)

go_repository(
    name = "com_github_go_resty_resty_v2",
    importpath = "github.com/go-resty/resty/v2",
    sum = "h1:me+K9p3uhSmXtrBZ4k9jcEAfJmuC8IivWHwaLZwPrFY=",
    version = "v2.7.0",  # November 3, 2021
)

go_repository(
    name = "com_github_kubemq_io_protobuf",
    commit = "cbaa3b00e2ea2f9f45dba49c6aaa28281d8b2e76",  # May 27, 2021
    importpath = "github.com/kubemq-io/protobuf",
)

go_repository(
    name = "com_github_nats_io_nuid",
    commit = "4b96681fa6d28dd0ab5fe79bac63b3a493d9ee94",  # April 9, 2019
    importpath = "github.com/nats-io/nuid",
)

go_repository(
    name = "org_uber_go_atomic",
    commit = "12f27ba2637fa0e13772a4f05fa46a5d18d53182",  # September 14, 2020
    importpath = "go.uber.org/atomic",
)

go_repository(
    name = "com_github_gogo_protobuf",
    importpath = "github.com/gogo/protobuf",
    version = "v1.3.2",  # January 10, 2021
)

# Needed by //a17/backend/common/logging
go_repository(
    name = "com_github_getsentry_raven_go",
    importpath = "github.com/getsentry/raven-go",
    tag = "v0.2.0",  # November 28, 2018
    # TODO (cdelguercio): migrate to sentry-go
)

go_repository(
    name = "com_github_pkg_errors",
    importpath = "github.com/pkg/errors",
    tag = "v0.9.1",  # January 14, 2020
)

go_repository(
    name = "com_github_certifi_gocertifi",
    importpath = "github.com/certifi/gocertifi",
    tag = "2021.04.29",
)

# Needed by //a17/backend/common/middleware
go_repository(
    name = "com_github_golang_jwt_jwt",
    importpath = "github.com/golang-jwt/jwt/v5",
    sum = "h1:1n1XNM9hk7O9mnQoNBGolZvzebBQ7p93ULHRc28XJUE=",
    version = "v5.0.0",  # April 17, 2023
)

# Needed by //a17/backend/common/pubsub
go_repository(
    name = "com_google_cloud_go",
    importpath = "cloud.google.com/go",
    tag = "v0.108.0",  # January 05, 2023
)

go_repository(
    name = "org_golang_google_api",
    commit = "ac7eb8f2e54c91697b33971d184eb155867cf1a2",  # January 04, 2023
    importpath = "google.golang.org/api",
)

go_repository(
    name = "com_github_googleapis_enterprise_certificate_proxy_client",
    importpath = "github.com/googleapis/enterprise-certificate-proxy",
    tag = "v0.2.1",  # December 6, 2022
)

go_repository(
    name = "org_golang_x_sync",
    importpath = "golang.org/x/sync",
    tag = "v0.1.0",  # September 29, 2022
)

go_repository(
    name = "org_golang_x_oauth2",
    importpath = "golang.org/x/oauth2",
    tag = "v0.4.0",  # January 04, 2023
)

go_repository(
    name = "org_golang_x_oauth2_google",
    importpath = "golang.org/x/oauth2",
    tag = "v0.4.0",  # January 04, 2023
)

go_repository(
    name = "io_opencensus_go",
    importpath = "go.opencensus.io",
    tag = "v0.24.0",  # November 03, 2022
)

go_repository(
    name = "com_github_golang_groupcache",
    commit = "41bb18bfe9da5321badc438f91158cd790a33aa3",  # March 23, 2021
    importpath = "github.com/golang/groupcache",
)

go_repository(
    name = "com_github_googleapis_gax_go",
    importpath = "github.com/googleapis/gax-go",
    tag = "v2.7.0",  # November 02, 2022
)

# Needed by //a17/backend/commmon/rabbitmq
go_repository(
    name = "com_github_rabbitmq_ampq091_go",
    importpath = "github.com/rabbitmq/amqp091-go",
    sum = "h1:GBFy5PpLQ5jSVVSYv8ecHGqeX7UTLYR4ItQbDCss9MM=",
    version = "v1.8.0",  # March 21, 2023
)

# Needed by //a17/backend/common/redis
go_repository(
    name = "com_github_bsm_redislock",
    importpath = "github.com/bsm/redislock",
    sum = "h1:osmvugkXGiLDEhzUPdM0EUtKpTEgLLuli4Ky2Z4vx38=",
    version = "v0.9.3",  # April 27, 2023
)

go_repository(
    name = "com_github_redis_go_redis_v9",
    importpath = "github.com/redis/go-redis/v9",
    sum = "h1:FC82T+CHJ/Q/PdyLW++GeCO+Ol59Y4T7R4jbgjvktgc=",
    version = "v9.0.4",  # May 1, 2023
)

go_repository(
    name = "github_com_cespare_xxhash_v2",
    importpath = "github.com/cespare/xxhash/v2",
    sum = "h1:DC2CZ1Ep5Y4k3ZQ899DldepgrayRUGE6BBZ/cd9Cj44=",
    version = "v2.2.0",  # December 3, 2022
)

go_repository(
    name = "com_github_dgryski_go_rendezvous",
    commit = "9f7001d12a5f0021fd3283525f888b5814ccee27",  # August 22, 2020
    importpath = "github.com/dgryski/go-rendezvous",
)

# Needed by //a17/backend/common/sds
go_repository(
    name = "com_github_robfig_cron",
    commit = "2315d5715e36303a941d907f038da7f7c44c773b",  # November 1, 2017
    importpath = "github.com/robfig/cron",
)

# Needed by com_github_robfig_cron.
go_repository(
    name = "com_github_hallas_stacko",
    commit = "f35fc1c0582692ba9aef59e275f0230c0992f9ae",  # August 23, 2014
    importpath = "github.com/hallas/stacko",
)

# Needed by //a17/backend/common/server
go_repository(
    name = "grpc_ecosystem_grpc_gateway",
    importpath = "github.com/grpc-ecosystem/grpc-gateway/v2",
    version = "v2.15.0",  # July 28, 2023
)

go_repository(
    name = "com_github_kazegusuri_grpc_panic_handler",
    commit = "093ec776affc30d46930f0ecb6ff6f41e2e04182",  # May 2, 2016
    importpath = "github.com/kazegusuri/grpc-panic-handler",
)

go_repository(
    name = "com_github_soheilhy_cmux",
    importpath = "github.com/soheilhy/cmux",
    tag = "v0.1.5",  # March 26, 2021
)

go_repository(
    name = "com_github_golang_glog",
    importpath = "github.com/golang/glog",
    tag = "v1.0.0",  # August 20, 2021
)

# Needed by //a17/backend/common/storage
go_repository(
    name = "com_github_aws_aws_sdk_go_v2",
    importpath = "github.com/aws/aws-sdk-go-v2",
    tag = "release-2023-01-09",  # January 09, 2023
)

go_repository(
    name = "com_github_aws_aws_sdk_go_v2_config",
    importpath = "github.com/aws/aws-sdk-go-v2/config",
    tag = "release-2023-01-05",  # January 05, 2023
)

go_repository(
    name = "com_github_aws_aws_sdk_go_v2_service_s3",
    importpath = "github.com/aws/aws-sdk-go-v2/service/s3",
    tag = "release-2023-01-05",  # January 05, 2023
)

go_repository(
    name = "com_github_aws_smithy_go",
    importpath = "github.com/aws/smithy-go",
    tag = "release-2022-12-02",  # December 02, 2022
)

# Needed by com_github_aws_aws_sdk_go_v2
go_repository(
    name = "com_github_jmespath_go_jmespath",
    commit = "3d4fd11601ddca248480565884e34e393313cd62",  # September 18, 2020
    importpath = "github.com/jmespath/go-jmespath",
)

# Needed by //a17/backend/common/timeseries
go_repository(
    name = "com_github_aws_aws_sdk_go_v2_service_timeseriesquery",
    importpath = "github.com/aws/aws-sdk-go-v2/service/timeseriesquery",
    tag = "release-2023-06-08",  # June 08, 2023
)

# Needed by //a17/backend/docker/service/buildJWKS.go
go_repository(
    name = "com_github_lestrrat_go_jwx_v2",
    importpath = "github.com/lestrrat-go/jwx",
    tag = "v2.0.16",  # October 30, 2023
)

go_repository(
    name = "com_github_lestrrat_go_option",
    importpath = "github.com/lestrrat-go/option",
    tag = "v1.0.1",  # December 15, 2022
)

go_repository(
    name = "com_github_lestrrat_go_iter",
    importpath = "github.com/lestrrat-go/iter",
    tag = "v1.0.2",  # March 28, 2022
)

go_repository(
    name = "com_github_lestrrat_go_blackmagic",
    importpath = "github.com/lestrrat-go/blackmagic",
    tag = "v1.0.2",  # November 7, 2022
)

go_repository(
    name = "com_github_lestrrat_go_httprc",
    importpath = "github.com/lestrrat-go/httprc",
    tag = "v1.0.4",  # July 18, 2022
)

go_repository(
    name = "com_github_lestrrat_go_httpcc",
    importpath = "github.com/lestrrat-go/httpcc",
    tag = "v1.0.1",  # March 28, 2022
)

# ------------------------------------------------------------------------------
http_archive(
    name = "com_github_OctoMap_octomap",
    build_file = "//third_party:octomap.BUILD",
    sha256 = "5f81c9a8cbc9526b2e725251cd3a829e5222a28201b394314002146d8b9214dd",
    strip_prefix = "octomap-1.9.0",
    urls = ["https://github.com/OctoMap/octomap/archive/v1.9.0.tar.gz"],
)

bind(
    name = "octomap",
    actual = "@com_github_OctoMap_octomap//:octomap",
)

# ------------------------------------------------------------------------------
http_archive(
    name = "eigen_archive",
    build_file = "//third_party:eigen.BUILD",
    sha256 = "94878cbfa27b0d0fbc64c00d4aafa137f678d5315ae62ba4aecddbd4269ae75f",
    strip_prefix = "eigen-eigen-67e894c6cd8f",
    urls = ["https://bitbucket.org/eigen/eigen/get/3.3.3.tar.gz"],
)

bind(
    name = "eigen",
    actual = "@eigen_archive//:eigen",
)

# GPL LICENSE DEPENDENCY - REMOVE ASAP
http_archive(
    name = "eigen_restricted_archive",
    build_file = "//third_party:eigen_restricted.BUILD",
    sha256 = "94878cbfa27b0d0fbc64c00d4aafa137f678d5315ae62ba4aecddbd4269ae75f",
    strip_prefix = "eigen-eigen-67e894c6cd8f",
    urls = ["https://bitbucket.org/eigen/eigen/get/3.3.3.tar.gz"],
)

bind(
    name = "eigen_restricted",
    actual = "@eigen_restricted_archive//:eigen_restricted",
)

# ------------------------------------------------------------------------------
http_archive(
    name = "com_github_sandstorm_io_capnproto",
    build_file = "//third_party:capnproto.BUILD",
    sha256 = "85210424c09693d8fe158c1970a2bca37af3a0424f02b263f566a1b8a5451a2d",
    strip_prefix = "capnproto-0.6.1",
    urls = ["https://github.com/sandstorm-io/capnproto/archive/v0.6.1.tar.gz"],
)

bind(
    name = "capnproto",
    actual = "@com_github_sandstorm_io_capnproto//:capnproto",
)

http_archive(
    name = "zmq_archive",
    build_file = "//third_party:zmq.BUILD",
    sha256 = "ea5eecbb9cc231a750535a7b74f887b18656014cc92601e8ab34734db799b796",
    strip_prefix = "libzmq-4.2.2",
    urls = ["https://github.com/zeromq/libzmq/archive/v4.2.2.tar.gz"],
)

bind(
    name = "zmq",
    actual = "@zmq_archive//:zmq",
)

http_archive(
    name = "util_linux_archive",
    build_file = "//third_party:util_linux.BUILD",
    sha256 = "2baabb17419faaf8325792b2bb02353078128dfa42b41b09f23e0ced011c8b6d",
    strip_prefix = "util-linux-2.29.2",
    urls = ["https://github.com/karelzak/util-linux/archive/v2.29.2.tar.gz"],
)

bind(
    name = "uuid",
    actual = "@util_linux_archive//:uuid",
)

http_archive(
    name = "spdlog_archive",
    build_file = "//third_party:spdlog.BUILD",
    sha256 = "d798a6ca19165f0a18a43938859359269f5a07fd8e0eb83ab8674739c9e8f361",
    strip_prefix = "spdlog-0.13.0",
    urls = ["https://github.com/gabime/spdlog/archive/v0.13.0.tar.gz"],
)

bind(
    name = "spdlog",
    actual = "@spdlog_archive//:spdlog",
)

http_archive(
    name = "azmq_archive",
    build_file = "//third_party:azmq.BUILD",
    sha256 = "42a2409fd471299841f83f5d50f5147914ea44165eae0c42d4a7cc6fbc8161a1",
    strip_prefix = "azmq-f1108af7cb0982c95f3c55ea73aab82345d97671",
    urls = ["https://github.com/zeromq/azmq/archive/f1108af7cb0982c95f3c55ea73aab82345d97671.tar.gz"],
)

bind(
    name = "azmq",
    actual = "@azmq_archive//:azmq",
)

# TODO(curtismuntz): This is actually unused unless specifically called... Do we need this at all?
# Maybe we should configure this toolchain to be clang, or set bazel.rc flags to point to this
# compiler to enforce the same gcc version as the cross-compiler.
http_archive(
    name = "org_linaro_components_toolchain_gcc",
    build_file = "//third_party:linaro_linux_gcc.BUILD",
    sha256 = "987941c9fffdf56ffcbe90e8984673c16648c477b537fcf43add22fa62f161cd",
    strip_prefix = "gcc-linaro-5.3.1-2016.05-x86_64_arm-linux-gnueabihf",
    url = "https://bazel-mirror.storage.googleapis.com/releases.linaro.org/components/toolchain/binaries/latest-5/arm-linux-gnueabihf/gcc-linaro-5.3.1-2016.05-x86_64_arm-linux-gnueabihf.tar.xz",
)

# Keep this compiler in sync with the kernel cross-compiler in the //kernel/cross_compile_kernel.sh
# file.
http_archive(
    name = "org_linaro_components_toolchain_gcc_aarch64",
    build_file = "//third_party:linaro_linux_gcc_aarch64.BUILD",
    sha256 = "8109232f6882d8a52c37a2062e66414465dcf20bdf6794ad6dc9188a1075f1bd",
    strip_prefix = "gcc-linaro-5.4.1-2017.05-x86_64_aarch64-linux-gnu",
    url = "https://releases.linaro.org/components/toolchain/binaries/5.4-2017.05/aarch64-linux-gnu/gcc-linaro-5.4.1-2017.05-x86_64_aarch64-linux-gnu.tar.xz",
)

new_local_repository(
    name = "qt",
    build_file = "//third_party:qt.BUILD",
    path = "/usr/include/x86_64-linux-gnu/qt5",
)

new_local_repository(
    name = "cuda_local",
    build_file = "//third_party:cuda.BUILD",
    path = "/usr/local/cuda",
)

bind(
    name = "cuda",
    actual = "@cuda_local//:cuda",
)

http_archive(
    name = "openssl_archive",
    build_file = "//third_party:openssl.BUILD",
    sha256 = "a3d3a7c03c90ba370405b2d12791598addfcafb1a77ef483c02a317a56c08485",
    url = "https://github.com/openssl/openssl/archive/OpenSSL_1_0_2l.tar.gz",
)

bind(
    name = "openssl",
    actual = "@openssl_archive//:openssl",
)

# Jan 19, 2022
http_archive(
    name = "googleapis_archive",
    build_file = "//third_party:googleapis.BUILD",
    sha256 = "df585427959ebb9949ca12a45888b50a5b8edcb2b270aba1800225d159a49ff6",
    strip_prefix = "googleapis-9fe00a1330817b5ce00919bf2861cd8a9cea1a00",
    urls = ["https://github.com/googleapis/googleapis/archive/9fe00a1330817b5ce00919bf2861cd8a9cea1a00.tar.gz"],
)

bind(
    name = "googleapis_protos",
    actual = "@googleapis_archive//:protos",
)

bind(
    name = "googleapis_py_proto",
    actual = "@googleapis_archive//:py_proto",
)

# master/HEAD as of July 2, 2018.
http_archive(
    name = "hfsm_archive",
    build_file = "//third_party:hfsm.BUILD",
    sha256 = "820113624d6424f76759eb7918b8b3a4f597fd3fae6c1f95a4a0de85c4b03a91",
    strip_prefix = "HFSM-fb35d57bf999847fca7188929a893b1e36399456",
    url = "https://github.com/andrew-gresyk/HFSM/archive/fb35d57bf999847fca7188929a893b1e36399456.tar.gz",
)

bind(
    name = "hfsm",
    actual = "@hfsm_archive//:hfsm",
)

http_archive(
    name = "cpplint_archive",
    build_file = "//third_party:cpplint.BUILD",
    sha256 = "952d19b925ebfefbd2a7724f48682dc42617965131f8058a4732fa167734c416",
    strip_prefix = "cpplint-5a38c3a6446914740884a976c9b5cded86b98e1c",
    urls = ["https://github.com/cpplint/cpplint/archive/5a38c3a6446914740884a976c9b5cded86b98e1c.tar.gz"],
)

http_archive(
    name = "pycodestyle_archive",
    build_file = "//third_party:pycodestyle.BUILD",
    sha256 = "559a8c0aab9140ff897ca834f26b6a0328a11278936c8c6ff6653401904b4540",
    strip_prefix = "pycodestyle-f4f6e556faea76809b70413fea9f496288d56713",
    urls = ["https://github.com/PyCQA/pycodestyle/archive/f4f6e556faea76809b70413fea9f496288d56713.tar.gz"],
)

# Aug 14, 2019
http_archive(
    name = "subpar",
    strip_prefix = "subpar-9fae6b63cfeace2e0fb93c9c1ebdc28d3991b16f",
    urls = ["https://github.com/google/subpar/archive/9fae6b63cfeace2e0fb93c9c1ebdc28d3991b16f.tar.gz"],
)

# Mavlink submodule version as of PX4 v1.8.0
# https://github.com/mavlink/c_library_v2/tree/033fa8e7a4a75a0c3f17cea57e3be8966e05f770
http_archive(
    name = "mavlink_archive",
    build_file = "//third_party:mavlink.BUILD",
    sha256 = "aa2e5ab3b860bceca352a599cef934538c9dd855e0c0c9d940baa6e03226e5e0",
    strip_prefix = "c_library_v2-033fa8e7a4a75a0c3f17cea57e3be8966e05f770",
    url = "https://github.com/mavlink/c_library_v2/archive/033fa8e7a4a75a0c3f17cea57e3be8966e05f770.tar.gz",
)

bind(
    name = "mavlink",
    actual = "@mavlink_archive//:mavlink",
)
