#include "fs_helper.hpp"
#include "encoding_conv.hpp"
#include "expected.hpp"
#include <variant>
#include <iostream>
#include <fstream>
#include <system_error>


static void test_known_path( )
{
	std::cout << "\nKnown path test\n";
	if( auto expected{ wh::fs::get_known_path( FOLDERID_CommonPrograms ) }; expected )
	{
		std::wstring path{ expected.value( ) };
		std::wcout << L"Known path test success. [" << path << L"]\n";
	}
	else
	{
		std::error_code ec{ expected.error( ) };
		std::cout << "Known path test failed: " << ec.message( ) << '\n';
	}
}

static void test_temp_path( )
{
	std::cout << "\nTemp path test\n";
	if( auto expected{ wh::fs::get_temp_path( ) }; expected )
	{
		std::wstring path{ expected.value( ) };
		std::wcout << L"Temp path test success. [" << path << L"]\n";
	}
	else
	{
		std::error_code ec{ expected.error( ) };
		std::cout << "Temp path test failed: " << ec.message( ) << '\n';
	}
}

static void test_current_path( )
{
	std::cout << "\nCurrent path test\n";
	if( auto expected{ wh::fs::get_current_dir( ) }; expected )
	{
		std::wstring path{ expected.value( ) };
		std::wcout << L"Current path test success. [" << path << L"]\n";
	}
	else
	{
		std::error_code ec{ expected.error( ) };
		std::cout << "Current path test failed: " << ec.message( ) << '\n';
	}
}

static void test_create_dir( std::wstring_view dir )
{
	std::cout << "\nCreate directory test\n";
	if( auto expected{ wh::fs::create_dir( dir ) }; expected )
		std::cout << "Create directory test success\n";	
	else
	{
		std::error_code ec{ expected.error( ) };
		std::cout << "Create directory test failed: " << ec.message( ) << '\n';
	}
}

static void test_dir_exists( std::wstring_view dir )
{
	std::cout << "\nDirectory exists test\n";
	if( wh::fs::dir_exists( dir ) )
		std::cout << "Directory exists test success\n";	
}

static void test_remove_dir( std::wstring_view dir )
{
	std::cout << "\nRemove directory test\n";
	if( auto expected{ wh::fs::remove_dir( dir ) }; expected )
		std::cout << "Remove directory test success\n";
	else
	{
		std::error_code ec{ expected.error( ) };
		std::cout << "Remove directory test failed: " << ec.message( ) << '\n';
	}
}

static void test_encoding( )
{
	std::wstring utf16str{ L"'Converted text'" };
	std::string utf8str;

	std::cout << "\nUTF16 to UTF8 test\n";
	if( auto expected_utf8{ wh::conv::to_utf8( utf16str ) }; expected_utf8 )
	{
		utf8str = expected_utf8.value( );
		std::cout << "UTF16 to UTF8 conversion success: [" << utf8str << "]\n";

		std::cout << "\nUTF8 to UTF16 test\n";
		if( auto expected_utf16{ wh::conv::to_utf16( utf8str ) };  expected_utf16 )
		{
			std::wstring utf16{ expected_utf16.value( ) };
			std::wcout << L"UTF8 to UTF16 conversion success: [" << utf16str << L"]\n";
		}
		else
		{
			std::error_code ec{ expected_utf16.error( ) };
			std::cout << "UTF8 to UTF16 test failed: " << ec.message( ) << '\n';
		}
	}
	else
	{
		std::error_code ec{ expected_utf8.error( ) };
		std::cout << "UTF16 to UTF8 test failed: " << ec.message( ) << '\n';
	}
}



int wmain( int argc, wchar_t* argv[ ] )
{
	test_known_path( );
	test_temp_path( );
	test_current_path( );
	if( argc > 1 )
	{
		std::wstring path{ argv[ 1 ] };
		test_create_dir( path );
		test_dir_exists( path );
		test_remove_dir( path );
	}
	test_encoding( );
	std::cin.get( );
	return 0;
}

