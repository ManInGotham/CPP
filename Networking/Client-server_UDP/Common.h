#pragma once

#pragma comment(lib, "ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS // for inet_addr

#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <iostream>
#include <string>

#define SERVER_PORT 50000

static void ShowWSError( const std::string& text, bool wsaGLE = true )
{
   std::cout << text;
   if( wsaGLE )
      std::cout << " (wsaGLE " << ::WSAGetLastError() << ")";
   std::cout << std::endl;
}

typedef unsigned int MSGINDEX;

// Common class which implements TCP\UDP connectivity and basic send/receive.
template<typename T>
class Connection
{
protected:
   SOCKADDR_IN m_address;
   WSADATA m_wsaData;
   SOCKET m_socket { INVALID_SOCKET };

public:
   Connection( int type, int protocol )
   {
      // Initialize the WS library. Version 2.0
      if( ::WSAStartup( MAKEWORD( 2, 2 ), &m_wsaData ) != 0 )
         ShowWSError( "WSAStartup() failed" );

      // Follow the standard server socket/bind/listen/accept sequence
      m_socket = ::socket( PF_INET, type, protocol );

      if( m_socket == INVALID_SOCKET )
         ShowWSError( "socket() failed" );

      // Prepare the socket address structure for binding the server socket to port number "reserved" for this service.
      m_address.sin_family = AF_INET;
      m_address.sin_port = htons( SERVER_PORT );
      m_address.sin_addr.s_addr = inet_addr( "127.0.0.1" );
   }

   virtual ~Connection()
   {
      if( m_socket != INVALID_SOCKET )
         if( ::closesocket( m_socket ) == SOCKET_ERROR )
            ShowWSError( "closesocket() failed" );

      WSACleanup();
   }

   void Send( _In_reads_bytes_( len ) const char FAR * buf, _In_ int len )
   {
      if( ::sendto( m_socket, buf, len, 0 /*flags*/, reinterpret_cast< SOCKADDR*>( &m_address ), sizeof( m_address ) ) == SOCKET_ERROR )
         ShowWSError( "sendto() failed" );
   }

   void Bind()
   {
      if( ::bind( m_socket, reinterpret_cast< SOCKADDR* >( &m_address ), sizeof( m_address ) ) == SOCKET_ERROR )
         ShowWSError( "bind() failed" );
   }

   template< typename T >
   void Recieve( T& message )
   {
      SOCKADDR_IN sender;
      int senderSize { sizeof( sender ) };
      if( ::recvfrom( m_socket, reinterpret_cast< char* >( &message ), sizeof( message ), 0 /*flags*/, reinterpret_cast< SOCKADDR* >( &sender ), &senderSize ) == SOCKET_ERROR )
         ShowWSError( "recvfrom() failed" );
   }
};

enum EventType : byte
{
   DIGITAL1, DIGITAL2, DIGITAL3, DIGITAL4, DIGITAL5, DIGITAL6, DIGITAL7, DIGITAL8,
   ANALOG1, ANALOG2, ANALOG3, ANALOG4,
   UNKNOWN
};

std::string EventTypes[] =
{
   "DIGITAL1", "DIGITAL2", "DIGITAL3", "DIGITAL4", "DIGITAL5", "DIGITAL6", "DIGITAL7", "DIGITAL8",
   "ANALOG1", "ANALOG2", "ANALOG3", "ANALOG4",
   "UNKNOWN"
};

struct GamePadEvent
{
   GamePadEvent() :
      GamePadEvent( EventType::UNKNOWN, 0 )
   { }

   GamePadEvent( EventType type, short value ) :
      eventType { type },
      eventValue { value },
      id { 0 }
   {}

   MSGINDEX id; // unique number for every sent event
   EventType eventType;
   short eventValue; // –32,768 to 32,767 for analog events; 0 (false) or 1 (true) for digital events

   static GamePadEvent GetRandom()
   {
      GamePadEvent event;
      event.eventType = static_cast< EventType >( std::rand() % 12 );

      if( event.eventType >= DIGITAL1 && event.eventType <= DIGITAL8 )
         event.eventValue = std::rand() % 2;
      else
         event.eventValue = SHRT_MIN / 2 + std::rand() % USHRT_MAX;

      return event;
   }
};


#define NUM_PACKETS 1000

