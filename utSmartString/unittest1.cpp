#include "stdafx.h"
#include "CppUnitTest.h"
#include "..\SmartString\SmartStrings.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace SmartStrings;
namespace utSmartString
{		
	TEST_CLASS(UnitTestsSmartStrings)
	{
	public:
		
		TEST_METHOD(GivenANewSmartStringWhenTheStringIsReturnedItsValueIsWhatWasInitialised)
		{
			SmartString s1( "Hello World" );
			Assert::AreEqual( (int)0, strcmp( "Hello World", s1.c_str( ) ) );
		}

		TEST_METHOD( GivenANewSmartStringWhenCopyConstructedBothStringsHaveTheSameValue )
		{
			SmartString s1( "Hello World" );
			SmartString s2( s1 );
			Assert::AreEqual( (int)0, strcmp( s2.c_str(), s1.c_str( ) ) );
			Assert::AreEqual( (int)2, s2.numRefs( ) );
		}

		TEST_METHOD( GivenASmartStringWithTwoReferencesWhenOneStringIsDestroyedTheNumberOfReferencesIsOne )
		{
			SmartString s1( "Hello World" );
			{
				SmartString s2( s1 );
				Assert::AreEqual( (int)0, strcmp( s2.c_str( ), s1.c_str( ) ) );
				Assert::AreEqual( (int)2, s2.numRefs( ) );
			}
			Assert::AreEqual( (int)1, s1.numRefs( ) );
		}

		TEST_METHOD( GivenASmartStringIsCopyAssignedThenTheNumberOfReferencesIsTwo )
		{
			SmartString s1( "Hello World" );
			
			{
				SmartString s2("Odd ball");
				s2 =  s1;
				Assert::AreEqual( (int)0, strcmp( s2.c_str( ), s1.c_str( ) ) );
				Assert::AreEqual( (int)2, s2.numRefs( ) );
			}
			Assert::AreEqual( (int)1, s1.numRefs( ) );
		}

		TEST_METHOD( GivenASmartStringWhenItsModifiedTheSmartPointerIsMarkedAsUnshareable )
		{
			SmartString s1( "Hello World" );
			char* p = s1[0];
			Assert::AreEqual( 'H', *p );
			Assert::IsFalse( s1.isShareable( ) );
			
		}

		TEST_METHOD( GivenASmartStringWithTwoReferencesWhenItsModifiedTheSmartPointerIsCopiedAndBothReferencesAreOne )
		{
			SmartString s1( "Hello World" );
			SmartString s2( s1 ); // we know this sets the reference count to 2

			char* p = s1[0];
			Assert::AreEqual( 'H', *p );
			Assert::AreEqual( (int)1, s2.numRefs() );
			Assert::AreEqual( (int)1, s1.numRefs( ) );

		}

		TEST_METHOD( GivenANonShareableSmartStringWhenUsedAsAnArgumentInACopyConstructorBothStringsHaveOnlyOneReference )
		{
			SmartString s1( "Hello World" );
			char* p = s1[1];

			SmartString s2( s1 ); // s1 is unshareable!
			Assert::AreEqual( 'e', *p );
			Assert::AreEqual( (int)1, s2.numRefs( ) );
			Assert::AreEqual( (int)1, s1.numRefs( ) );

		}

		TEST_METHOD( GivenANonShareableSmartStringWhenUsedAsAssignmentToAnExistingSmartStringBothStringsHaveOnlyOneReference )
		{
			SmartString s1( "Hello World" );
			char* p = s1[1];

			SmartString s2( "Some temporary value" ); // s2
			s2 = s1; // s1 is not shareable
			Assert::AreEqual( (int)1, s2.numRefs( ) );
			Assert::AreEqual( (int)1, s1.numRefs( ) );

		}

		TEST_METHOD( GivenANewConstantSmartStringWhenAConstantReadOnlyPointerIsUsedTheStringRemainsShareable )
		{
			const SmartString s1( "Hello World" );
			auto p = s1[1];

			Assert::IsTrue( s1.isShareable());
		}

	};
}