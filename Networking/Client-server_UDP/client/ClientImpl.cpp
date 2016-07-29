//////////////////////////////////////////////////////////////////////////
// Author: Gleb Karapish
// 
// This module implements a client to simulate UDP packets traffic and 
// packet losses.
//////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <climits>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <thread>

#include "../Common.h"

// To get extra layer of reliability over UDP I decided to resend the same message after a short wait again.
// The server will need to keep accounting for already seen events and ignore them.
// Unfortunate effect is that the total UDP traffic is doubled.
class Client
{
private:
   Connection< GamePadEvent > m_UDPtoServer { SOCK_DGRAM, IPPROTO_UDP };
   static MSGINDEX m_msgIndex;

public:
   void Send( GamePadEvent event, MSGINDEX index = 0 )
   {
      const bool sendingFirstTime = index == 0;
      event.id = sendingFirstTime ? m_msgIndex++ : index;

      if( !LosePacket() ) // simulate packets loss
         m_UDPtoServer.Send( reinterpret_cast< char* >( &event ), sizeof( event ) );

      if( sendingFirstTime /*need to re-send after a short wait*/ )
         WaitAndSendAsync( event );
   }

private:
   std::thread WaitAndSendAsync( GamePadEvent event, unsigned int waitMsec = 15 )
   {
      auto thread = std::thread( [waitMsec, event, this]()
      {
         std::this_thread::sleep_for( std::chrono::milliseconds( waitMsec ) );
         Send( event, event.id );
      } );
      thread.detach();
      return thread;
   };

   bool LosePacket() const
   {
      return std::rand() > std::rand();
   }
};

MSGINDEX Client::m_msgIndex { 1 };

void main()
{
   std::srand( static_cast< unsigned int >( std::time( 0 ) ) );

   Client client;
   for( auto i = 1; i <= NUM_PACKETS; ++i )
   {
      client.Send( GamePadEvent::GetRandom() );
      std::this_thread::sleep_for( std::chrono::microseconds( 14 ) );
   }
}