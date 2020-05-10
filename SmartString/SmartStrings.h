#pragma once
#include <stdio.h>
#include <string.h>

namespace SmartStrings
{
	///<summary>
	///shared _ptr is a template class to allow me to simplify the 
	///management of the string_ptr contained in the String.
	/// when the shared_ptr is assigned or copied it will take care 
	/// of the manipulation of the counter, deletion etc.
	/// this implementation assumes the interface provided by
	/// 'ReferenceCounted'' is available. So the TYPE must inherit
	/// from ReferenceCounted or provide the equivalent functionality.
	template <typename PTR>
	class shared_ptr
	{
	private:
		PTR* _thePtr;
		///<summary>
		/// init is called when _thePtr is assigned a new value
		/// from the constructors and assignment operators.
		/// if the pointer is shareable we can add a reference
		/// if not we must make a deep copy before adding a 
		/// reference to the new object.
		///</summary>
		void init( void )
		{
			if ( nullptr == _thePtr ) // nowt to do
				return;

			if ( !_thePtr->isShareable( ) ) 
			{
				// make a deep copy - check the copy
				// constructor of the PTR type, in this case 
				// a string_ptr
				this->_thePtr = new PTR( *_thePtr );
			}
			
			_thePtr->addReference( );

		}

	public:
		///<summary>basic constructor</summary>
		shared_ptr( PTR* ptr = nullptr ) : _thePtr( ptr )	{
			init( );
		}

		///<summary>
		/// When the shared_ptr destructor is called it decrements references
		///</summary>
		~shared_ptr( ) {
			if(_thePtr ) _thePtr->removeReference( );
		}

		///<summary>copy constructor assigns the pointee from the class being copied.</summary>
		shared_ptr( const shared_ptr& shared ) : _thePtr( shared._thePtr )
		{
			init( );
		}

		shared_ptr& operator=( const shared_ptr& shared );

		/// allows access to the pointer
		PTR* get( void ) const { return _thePtr; }

		// standard accessors for a ptr
		PTR *operator->( ) const {
			return _thePtr;
		}

		PTR &operator*() const {
			return *_thePtr;
		}

	};


	template<typename PTR>
	shared_ptr<PTR>& shared_ptr<PTR>::operator=( const shared_ptr& shared ) 
	{
		// check assignment to self
		if ( _thePtr == shared._thePtr ) return *this;

		// we need to decrement our reference to our existing reference counted ptr
		// if we have one, so store the old pointee and then re-assign the pointee
		// of this class
		PTR* oldpointee = _thePtr;
		_thePtr = shared._thePtr;

		// run our standard initiation. This checks to see if the pointee we are 
		// copying is shareable and if so just adds a reference. If not it makes
		// a deep copy.
		init( );

		// if the oldpointee is not null, update it's references
		if ( nullptr != oldpointee ) { oldpointee->removeReference( ); }

		return *this;
	}

	///<summary>
	/// ReferenceCounted stores all the reference counting code
	///</summmary>
	class RCObject
	{
	private:
		int _refCount;
		bool _bShareable;
	public:
		RCObject( ) : _refCount(0 ), _bShareable(true) {}
		virtual ~RCObject( ) = 0; // must be used as base class

		///<summary>
		///We don't copy string_ptr or assign them. 
		///We do copy assign things that point to them. 
		///if we did assign or copy construct, the correct thing to
		///do is nothing, as the external references haven't changed?
		///</summary>
		RCObject( const RCObject& ) {}
		RCObject& operator=( const RCObject& ) {
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

		void addReference( void ) {
			 ++_refCount;
		}

		void removeReference( void ) {
			if ( --_refCount == 0 ) delete this;
		}
	};

	// pure virtual must have implementation
	RCObject::~RCObject( ) {}

	///<summary>
	///The main object that users will create. This maintains the underlying string
	///pointer, and manipulation of the reference count based on operations against
	///the string.
	///</summary>
	class String
	{
	private:
		///<summary>our string implementation is primate</summary>
		struct string_ptr : public RCObject
		{
			explicit string_ptr( const char* i_pszString )
			{
				init( i_pszString );
			}

			string_ptr( const string_ptr& rhs )
			{
				init( rhs._pszStr );
			}


			~string_ptr( ) {
				delete[] _pszStr;
			}

			void init( const char* value )
			{
				_pszStr = new char[strlen( value ) + 1];
				strcpy_s( _pszStr, strlen( value ) + 1, value );
			}

			char* _pszStr;
		};


	public:
		explicit String( const char* i_pszString = "" );
		String( const String& smart ) = default;
		String& operator=( const String& smart ) = default;

		~String( ) {}

		///<param>int index - the char position you want to return</param>
		char* operator[]( std::size_t index ) {
			
			check_out_of_range( index );

			// I need to check if my current pointer is shared, if so
			// I need to make a deep copy of the string.
			// the resulting string_ptr member is marked as unshareable
			if ( isShared() ) {
				// make a deep copy...
				//m_psstr->DecrementReferences( ); // decrement the number of references against the current string ptr
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
			return this->m_psstr->getRefCount( );
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
		shared_ptr<string_ptr> m_psstr;

		string_ptr* create_string_ptr( const char* i_pszString ) {
			string_ptr* pstring = new string_ptr( i_pszString );
			
			return pstring;
		}

		void check_out_of_range( size_t index ) const {
			if ( index >= strlen( m_psstr->_pszStr ) ) {
				throw std::out_of_range( "string index out of bounds" );
			}
		}
	};

	///<summary>Default constructor, takes a char* input </summary>
	String::String( const char* i_pszString )
	{
		m_psstr = create_string_ptr( i_pszString );
	}


}