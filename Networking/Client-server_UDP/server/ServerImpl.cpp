//////////////////////////////////////////////////////////////////////////
// Author: Gleb Karapish
// 
// This module implements a server which accepts messages and ignores duplicated 
// messages.
//////////////////////////////////////////////////////////////////////////

#include <set>

#include "..\Common.h"

class Server
{
private:
   std::set< MSGINDEX > m_ids; // Already processed IDs
   Connection< GamePadEvent > m_server { SOCK_DGRAM, IPPROTO_UDP };

public:
   Server()
   {
      m_server.Bind();
   }

   void Run()
   {
      std::cout << "Server is launched. Launch the client" << std::endl;

      GamePadEvent message;
      while( message.id < NUM_PACKETS )
      {
         m_server.Recieve( message );
         std::cout << "Msg " << message.id << "  " << EventTypes[ message.eventType - EventType::DIGITAL1 ] << " = " << message.eventValue;

         const bool duplicated = m_ids.find( message.id ) != m_ids.end();
         std::cout << ( duplicated ? " (ignore) " : std::string() ) << std::endl;

         if( !duplicated )
            m_ids.insert( message.id );
      }

      std::cout << "Server is stopped" << std::endl;
   }

   MSGINDEX CountLoss() const
   {
      MSGINDEX index { 0 }, count { 0 };
      for( const auto& item : m_ids )
      {
         if( item - index > 1 )
            count++;
         index = item;
      }

      return count;
   }
};

void main()
{
   Server server;
   server.Run();

   std::cout << "# lost packets " << server.CountLoss() << std::endl;
   std::cout << "Press any buttons to exit" << std::endl;

   int fake;
   std::cin >> fake;
}