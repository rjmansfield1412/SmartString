#pragma once
#include <stdio.h>
#include <string.h>

namespace SmartStrings
{
	///<summary>
	/// ReferenceCounted stores all the reference counting code
	///</summmary>
	class ReferenceCounted
	{
	private:
		int _refCount;
		bool _bShareable;
	public:
		ReferenceCounted( ) : _refCount(0 ), _bShareable(true) {}
		virtual ~ReferenceCounted( ) = 0; // must be used as base class

		///<summary>
		///We don't copy string_ptr or assign them. 
		///We do copy assign things that point to them. 
		///if we did assign or copy construct, the correct thing to
		///do is nothing, as the external references haven't changed?
		///</summary>
		ReferenceCounted( const ReferenceCounted& ) {}
		ReferenceCounted& operator=( const ReferenceCounted& ) {
			return *this;
		}

		bool isShareable( void ) const {
			return _bShareable;
		}

		void markUnshareable( void ) {
			_bShareable = false;
		}

		int getRefCount( void ) const {
			return _refCount;
		}

		void IncrementReferences( void ) {
			 ++_refCount;
		}

		void DecrementReferences( void ) {
			if ( --_refCount == 0 ) delete this;
		}
	};

	// pure virtual must have implementation
	ReferenceCounted::~ReferenceCounted( ) {}

	///<summary>
	///The main object that users will create. This maintains the underlying string
	///pointer, and manipulation of the reference count based on operations against
	///the string.
	///</summary>
	class String
	{
	private:
		///<summary>our string implementation is primate</summary>
		struct string_ptr : public ReferenceCounted
		{
			explicit string_ptr( const char* i_pszString )
			{
				_pszStr = new char[strlen( i_pszString ) + 1];
				strcpy_s( _pszStr, strlen( i_pszString ) + 1, i_pszString );
			}

			~string_ptr( ) {
				delete[] _pszStr;
			}

			char* _pszStr;
		};


	public:
		explicit String( const char* i_pszString = "" );
		String( const String& smart );
		String& operator=( const String& smart );

		~String( ) 
		{
			m_psstr->DecrementReferences( );
		}

		///<param>int index - the char position you want to return</param>
		char* operator[]( std::size_t index ) {
			
			check_out_of_range( index );

			// I need to check if my current pointer is shared, if so
			// I need to make a deep copy of the string.
			// the resulting string_ptr member is marked as unshareable
			if ( isShared() ) {
				// make a deep copy...
				m_psstr->DecrementReferences( ); // decrement the number of references against the current string ptr
				// make a deep copy of the string value, creating a new string_ptr
				m_psstr = create_string_ptr( m_psstr->_pszStr );
			}

			m_psstr->markUnshareable( );

			return &m_psstr->_pszStr[index];
		}

		///<summary>const version of operator []</summary>
		const char* operator[]( size_t index ) const {
			check_out_of_range( index );

			return &m_psstr->_pszStr[index];
		}

		int numRefs( void ) const {
			return this->m_psstr->getRefCount();
		}

		bool isShareable( void ) const {
			return this->m_psstr->isShareable();
		}

		bool isShared( void ) const {
			return numRefs( ) > 1;
		}

		const char* c_str( void ) const {
			return m_psstr->_pszStr;
		}
	private:
		string_ptr* m_psstr;

		void init( string_ptr* p ) {
			this->m_psstr = p;
			p->IncrementReferences();
		}

		string_ptr* create_string_ptr( const char* i_pszString ) {
			string_ptr* pstring = new string_ptr( i_pszString );
			pstring->IncrementReferences( );

			return pstring;
		}

		void check_out_of_range( size_t index ) const {
			if ( index >= strlen( m_psstr->_pszStr ) ) {
				throw std::out_of_range( "string index out of bounds" );
			}
		}
	};

	String::String( const String& sstring )
	{
		// the copy constructor just assigns the m_psstr
		// and increments the counter..
		// if the string input is not shareable, I need t make a deep copy
		if ( !sstring.isShareable( ) ) {
			this->m_psstr = create_string_ptr( sstring.c_str( ) );
		}
		else {
			// if it is shareable just assign the string_ptr to the rhs
			// and increment the number of references, what init does!
			init( sstring.m_psstr );
		}
	}

	///<summary>
	/// Assignment operator is similar to copy however we 
	/// need to adjust our existing string_ptr member before assignment
	///</summary>
	String& String::operator=( const String& smart )
	{
		/// check assignment to self
		if ( this->m_psstr == smart.m_psstr ) {
			return *this;
		}
		// if no one else if referencing our pointer we can delete it!
		m_psstr->DecrementReferences( );

		if ( smart.isShareable( ) ) {
			init( smart.m_psstr );
		}
		else {
			m_psstr = create_string_ptr( smart.c_str( ) );
		}

		return *this;
	}
	///<summary>Default constructor, takes a char* input </summary>
	String::String( const char* i_pszString )
	{
		m_psstr = create_string_ptr( i_pszString );
	}


}