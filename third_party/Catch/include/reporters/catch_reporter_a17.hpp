/*
 *  Created by Phil Nash on 19th December 2014
 *  Copyright 2014 Two Blue Cubes Ltd. All rights reserved.
 *
 *  Distributed under the Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef TWOBLUECUBES_CATCH_REPORTER_A17_HPP_INCLUDED
#define TWOBLUECUBES_CATCH_REPORTER_A17_HPP_INCLUDED

// Don't #include any Catch headers here - we can assume they are already
// included before this header.
// This is not good practice in general but is necessary in this case so this
// file can be distributed as a single header that works with the main
// Catch single header.

#include <cstring>
#include "catch_reporter_bases.hpp"

#include "../internal/catch_reporter_registrars.hpp"
#include "../internal/catch_console_colour.hpp"

#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wpadded"
#   pragma clang diagnostic ignored "-Wc++98-compat"
#   pragma clang diagnostic ignored "-Wc++98-compat-pedantic"
#endif

namespace Catch {

    struct A17Reporter : StreamingReporterBase {
        A17Reporter( ReporterConfig const& _config )
        :   StreamingReporterBase( _config ),
            m_headerPrintedForThisSection( false )
        { }

        static std::string escape( std::string const& str ) {
            std::string escaped = str;
            replaceInPlace( escaped, "|", "||" );
            replaceInPlace( escaped, "'", "|'" );
            replaceInPlace( escaped, "\n", "|n" );
            replaceInPlace( escaped, "\r", "|r" );
            replaceInPlace( escaped, "[", "|[" );
            replaceInPlace( escaped, "]", "|]" );
            return escaped;
        }
        virtual ~A17Reporter() CATCH_OVERRIDE;

        static std::string getDescription() {
            return "Reports test results as a17 styling";
        }

        virtual void skipTest( TestCaseInfo const& testInfo ) CATCH_OVERRIDE {
            stream  << "[ SKIPPING ]"
                    << testInfo.name << "'";
            if( testInfo.isHidden() )
                stream << " hidden test";
            else
                stream << " test skipped because it didn|'t match the test spec";
            stream << "\n";
        }

        virtual void noMatchingTestCases( std::string const& /* spec */ ) CATCH_OVERRIDE {}

        virtual void testGroupStarting( GroupInfo const& groupInfo ) CATCH_OVERRIDE {
            StreamingReporterBase::testGroupStarting( groupInfo );
            stream << "Starting new test group: " << groupInfo.name << "\n";
        }
        virtual void testGroupEnded( TestGroupStats const& testGroupStats ) CATCH_OVERRIDE {
            printTotals( testGroupStats.totals );
            stream << "\n" << std::endl;
            StreamingReporterBase::testGroupEnded( testGroupStats );
        }

        virtual void assertionStarting( AssertionInfo const& ) CATCH_OVERRIDE {
        }

        // virtual bool assertionEnded( AssertionStats const& assertionStats ) CATCH_OVERRIDE {
        //     AssertionResult const& result = assertionStats.assertionResult;
        //     if( !result.isOk() ) {

        //         std::ostringstream msg;
        //         if( !m_headerPrintedForThisSection )
        //             printSectionHeader( msg );
        //         m_headerPrintedForThisSection = true;

        //         msg << result.getSourceInfo() << "\n";

        //         switch( result.getResultType() ) {
        //             case ResultWas::ExpressionFailed:
        //                 msg << "expression failed";
        //                 break;
        //             case ResultWas::ThrewException:
        //                 msg << "unexpected exception";
        //                 break;
        //             case ResultWas::FatalErrorCondition:
        //                 msg << "fatal error condition";
        //                 break;
        //             case ResultWas::DidntThrowException:
        //                 msg << "no exception was thrown where one was expected";
        //                 break;
        //             case ResultWas::ExplicitFailure:
        //                 msg << "explicit failure";
        //                 break;

        //             // We shouldn't get here because of the isOk() test
        //             case ResultWas::Ok:
        //             case ResultWas::Info:
        //             case ResultWas::Warning:

        //             // These cases are here to prevent compiler warnings
        //             case ResultWas::Unknown:
        //             case ResultWas::FailureBit:
        //             case ResultWas::Exception:
        //                 CATCH_NOT_IMPLEMENTED;
        //         }
        //         if( assertionStats.infoMessages.size() == 1 )
        //             msg << " with message:";
        //         if( assertionStats.infoMessages.size() > 1 )
        //             msg << " with messages:";
        //         for( std::vector<MessageInfo>::const_iterator
        //                 it = assertionStats.infoMessages.begin(),
        //                 itEnd = assertionStats.infoMessages.end();
        //             it != itEnd;
        //             ++it )
        //             msg << "\n  \"" << it->message << "\"";


        //         if( result.hasExpression() ) {
        //             msg <<
        //                 "\n  " << result.getExpressionInMacro() << "\n"
        //                 "with expansion:\n" <<
        //                 "  " << result.getExpandedExpression() << "\n";
        //         }

        //         stream << Colour( Colour::ResultSuccess ) << "##teamcity[testFailed";
        //         stream << " name='" << escape( currentTestCaseInfo->name )<< "'"
        //             << " message='" << escape( msg.str() ) << "'"
        //             << "]\n";
        //     }
        //     return true;
        // }
        virtual bool assertionEnded( AssertionStats const& _assertionStats ) CATCH_OVERRIDE {
            AssertionResult const& result = _assertionStats.assertionResult;

            bool printInfoMessages = true;

            // Drop out if result was successful and we're not printing those
            if( !m_config->includeSuccessfulResults() && result.isOk() ) {
                if( result.getResultType() != ResultWas::Warning )
                    return false;
                printInfoMessages = false;
            }

            // lazyPrint();

            AssertionPrinter printer( stream, _assertionStats, printInfoMessages );
            printer.print();
            stream << std::endl;
            return true;
        }

        virtual void sectionStarting( SectionInfo const& sectionInfo ) CATCH_OVERRIDE {
            m_headerPrintedForThisSection = false;
            StreamingReporterBase::sectionStarting( sectionInfo );
            if(sectionInfo.name != curTestName) { 
                stream << Colour( Colour::Yellow ) << "< SECTION  > ";
                stream << sectionInfo.name << "\n";
            }
        }

        virtual void testCaseStarting( TestCaseInfo const& testInfo ) CATCH_OVERRIDE {
            StreamingReporterBase::testCaseStarting( testInfo );
            stream << Colour( Colour::ResultSuccess ) << "[ RUN      ] ";
            stream << testInfo.name << "\n";
            curTestName = testInfo.name;
        }

        virtual void testCaseEnded( TestCaseStats const& testCaseStats ) CATCH_OVERRIDE {
            StreamingReporterBase::testCaseEnded( testCaseStats );
            // if( !testCaseStats.stdOut.empty() )
            //     stream << "##teamcity[testStdOut name='"
            //         << escape( testCaseStats.testInfo.name )
            //         << "' out='" << escape( testCaseStats.stdOut ) << "']\n";
            // if( !testCaseStats.stdErr.empty() )
            //     stream << "##teamcity[testStdErr name='"
            //         << escape( testCaseStats.testInfo.name )
            //         << "' out='" << escape( testCaseStats.stdErr ) << "']\n";
            stream << Colour( Colour::ResultSuccess ) << "[     DONE ] ";
            stream << testCaseStats.testInfo.name << "\n";
        }

    private:
        void printSectionHeader( std::ostream& os ) {
            assert( !m_sectionStack.empty() );

            if( m_sectionStack.size() > 1 ) {
                os << getLineOfChars<'-'>() << "\n";

                std::vector<SectionInfo>::const_iterator
                it = m_sectionStack.begin()+1, // Skip first section (test case)
                itEnd = m_sectionStack.end();
                for( ; it != itEnd; ++it )
                    printHeaderString( os, it->name );
                os << getLineOfChars<'-'>() << "\n";
            }

            SourceLineInfo lineInfo = m_sectionStack.front().lineInfo;

            if( !lineInfo.empty() )
                os << lineInfo << "\n";
            os << getLineOfChars<'.'>() << "\n\n";
        }

        // if string has a : in first line will set indent to follow it on
        // subsequent lines
        void printHeaderString( std::ostream& os, std::string const& _string, std::size_t indent = 0 ) {
            std::size_t i = _string.find( ": " );
            if( i != std::string::npos )
                i+=2;
            else
                i = 0;
            os << Text( _string, TextAttributes()
                           .setIndent( indent+i)
                           .setInitialIndent( indent ) ) << "\n";
        }

        struct SummaryColumn {

            SummaryColumn( std::string const& _label, Colour::Code _colour )
            :   label( _label ),
                colour( _colour )
            {}
            SummaryColumn addRow( std::size_t count ) {
                std::ostringstream oss;
                oss << count;
                std::string row = oss.str();
                for( std::vector<std::string>::iterator it = rows.begin(); it != rows.end(); ++it ) {
                    while( it->size() < row.size() )
                        *it = " " + *it;
                    while( it->size() > row.size() )
                        row = " " + row;
                }
                rows.push_back( row );
                return *this;
            }

            std::string label;
            Colour::Code colour;
            std::vector<std::string> rows;

        };

        void printTotals( Totals const& totals ) {
            if( totals.testCases.total() == 0 ) {
                stream << Colour( Colour::Warning ) << "No tests ran\n";
            }
            else if( totals.assertions.total() > 0 && totals.testCases.allPassed() ) {
                stream << Colour( Colour::ResultSuccess ) << "All tests passed";
                stream << " ("
                        << pluralise( totals.assertions.passed, "assertion" ) << " in "
                        << pluralise( totals.testCases.passed, "test case" ) << ")"
                        << "\n";
            }
            else {

                std::vector<SummaryColumn> columns;
                columns.push_back( SummaryColumn( "", Colour::None )
                                        .addRow( totals.testCases.total() )
                                        .addRow( totals.assertions.total() ) );
                columns.push_back( SummaryColumn( "passed", Colour::Success )
                                        .addRow( totals.testCases.passed )
                                        .addRow( totals.assertions.passed ) );
                columns.push_back( SummaryColumn( "failed", Colour::ResultError )
                                        .addRow( totals.testCases.failed )
                                        .addRow( totals.assertions.failed ) );
                columns.push_back( SummaryColumn( "failed as expected", Colour::ResultExpectedFailure )
                                        .addRow( totals.testCases.failedButOk )
                                        .addRow( totals.assertions.failedButOk ) );

                printSummaryRow( "test cases", columns, 0 );
                printSummaryRow( "assertions", columns, 1 );
            }
        }

        void printSummaryRow( std::string const& label, std::vector<SummaryColumn> const& cols, std::size_t row ) {
            for( std::vector<SummaryColumn>::const_iterator it = cols.begin(); it != cols.end(); ++it ) {
                std::string value = it->rows[row];
                if( it->label.empty() ) {
                    stream << label << ": ";
                    if( value != "0" )
                        stream << value;
                    else
                        stream << Colour( Colour::Warning ) << "- none -";
                }
                else if( value != "0" ) {
                    stream  << Colour( Colour::LightGrey ) << " | ";
                    stream  << Colour( it->colour )
                            << value << " " << it->label;
                }
            }
            stream << "\n";
        }

        class AssertionPrinter {
            void operator= ( AssertionPrinter const& );
        public:
            AssertionPrinter( std::ostream& _stream, AssertionStats const& _stats, bool _printInfoMessages )
            :   stream( _stream ),
                stats( _stats ),
                result( _stats.assertionResult ),
                colour( Colour::None ),
                message( result.getMessage() ),
                messages( _stats.infoMessages ),
                printInfoMessages( _printInfoMessages )
            {
                switch( result.getResultType() ) {
                    case ResultWas::Ok:
                        colour = Colour::Success;
                        passOrFail = "PASSED";
                        //if( result.hasMessage() )
                        if( _stats.infoMessages.size() == 1 )
                            messageLabel = "with message";
                        if( _stats.infoMessages.size() > 1 )
                            messageLabel = "with messages";
                        break;
                    case ResultWas::ExpressionFailed:
                        if( result.isOk() ) {
                            colour = Colour::Success;
                            passOrFail = "FAILED - but was ok";
                        }
                        else {
                            colour = Colour::Error;
                            passOrFail = "FAILED";
                        }
                        if( _stats.infoMessages.size() == 1 )
                            messageLabel = "with message";
                        if( _stats.infoMessages.size() > 1 )
                            messageLabel = "with messages";
                        break;
                    case ResultWas::ThrewException:
                        colour = Colour::Error;
                        passOrFail = "FAILED";
                        messageLabel = "due to unexpected exception with message";
                        break;
                    case ResultWas::FatalErrorCondition:
                        colour = Colour::Error;
                        passOrFail = "FAILED";
                        messageLabel = "due to a fatal error condition";
                        break;
                    case ResultWas::DidntThrowException:
                        colour = Colour::Error;
                        passOrFail = "FAILED";
                        messageLabel = "because no exception was thrown where one was expected";
                        break;
                    case ResultWas::Info:
                        messageLabel = "info";
                        break;
                    case ResultWas::Warning:
                        messageLabel = "warning";
                        break;
                    case ResultWas::ExplicitFailure:
                        passOrFail = "FAILED";
                        colour = Colour::Error;
                        if( _stats.infoMessages.size() == 1 )
                            messageLabel = "explicitly with message";
                        if( _stats.infoMessages.size() > 1 )
                            messageLabel = "explicitly with messages";
                        break;
                    // These cases are here to prevent compiler warnings
                    case ResultWas::Unknown:
                    case ResultWas::FailureBit:
                    case ResultWas::Exception:
                        passOrFail = "** internal error **";
                        colour = Colour::Error;
                        break;
                }
            }

            void print() const {
                printResultType();
                printSourceInfo();
                printOriginalExpression();
                if( stats.totals.assertions.total() > 0 ) {
                    if( result.isOk() )
                        stream << "\n";
                    printReconstructedExpression();
                }
                else {
                    stream << "\n";
                }
                printMessage();
            }

        private:
            void printResultType() const {
                if( !passOrFail.empty() ) {
                    Colour colourGuard( colour );
                    stream << "[  " << passOrFail << "  ] ";
                }
            }
            void printOriginalExpression() const {
                if( result.hasExpression() ) {
                    Colour colourGuard( Colour::OriginalExpression );
                    stream  << "  ";
                    stream << result.getExpressionInMacro();
                    stream << "\n";
                }
            }
            void printReconstructedExpression() const {
                if( result.hasExpandedExpression() ) {
                    Colour colourGuard( Colour::ReconstructedExpression );
                    stream << Text( result.getExpandedExpression(), TextAttributes().setIndent(2) );// << "\n";
                }
            }
            void printMessage() const {
                if( !messageLabel.empty() )
                    stream << messageLabel << ":" << "\n";
                for( std::vector<MessageInfo>::const_iterator it = messages.begin(), itEnd = messages.end();
                        it != itEnd;
                        ++it ) {
                    // If this assertion is a warning ignore any INFO messages
                    if( printInfoMessages || it->type != ResultWas::Info )
                        stream << Text( it->message, TextAttributes().setIndent(2) ) << "\n";
                }
            }
            void printSourceInfo() const {
                Colour colourGuard( Colour::FileName );
                stream << result.getSourceInfo() << "\n";
            }

            std::ostream& stream;
            AssertionStats const& stats;
            AssertionResult const& result;
            Colour::Code colour;
            std::string passOrFail;
            std::string messageLabel;
            std::string message;
            std::vector<MessageInfo> messages;
            bool printInfoMessages;
        };

    private:
        bool m_headerPrintedForThisSection;
        std::string curTestName;
    };

#ifdef CATCH_IMPL
    A17Reporter::~A17Reporter() {}
#endif

    INTERNAL_CATCH_REGISTER_REPORTER( "a17", A17Reporter )

} // end namespace Catch

#ifdef __clang__
#   pragma clang diagnostic pop
#endif

#endif // TWOBLUECUBES_CATCH_REPORTER_A17_HPP_INCLUDED
