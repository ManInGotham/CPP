#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream.hpp>
#include <intrin.h>

namespace io = boost::iostreams;

using boost::asio::ip::tcp;
using namespace boost::algorithm;
using namespace std;

typedef  unsigned char BYTE;

typedef struct {
  BYTE seq[4];
  BYTE chk[4];
  BYTE len[4];
} PACKET_HEADER;

int main(){

  boost::asio::io_service io_service;
  tcp::resolver resolver(io_service);
  tcp::socket socket(io_service);

  boost::asio::connect(
    socket, resolver.resolve(
      tcp::resolver::query("challenge2.airtime.com", "2323")));

  std::string handshake("WHORU:3711870633");
  boost::asio::read(socket, boost::asio::buffer(&handshake[0], handshake.size()));
  erase_head(handshake, 6);

  std::ostringstream stringStream;
  stringStream << "IAM:" << handshake << ":" << "karapish@outlook.com:at\n";
  handshake = stringStream.str();  
  socket.send(boost::asio::buffer(&handshake[0], handshake.size()));

  std::string success("SUCCESS:701925");
  boost::asio::read(socket, boost::asio::buffer(&success[0], success.size()));
  erase_head(success, 9);
                
  std::vector<unsigned char> pcm;

  io::stream_buffer<io::file_sink> buf("file.bin");
  std::ostream                     out(&buf);

  while (true) {
    unsigned int buf[3];
    auto read = boost::asio::read(socket, boost::asio::buffer(buf), boost::asio::transfer_exactly(12));
    unsigned int seq = buf[0];
    unsigned int chk = buf[1];
    unsigned int len = buf[2];

    seq = _byteswap_ulong(seq);
    bool missing = static_cast<bool>(!(len % 4));

    /*for (unsigned int i = 1; i <= len; ++i)*/
    {
      signed int buf2[50000];
      auto read = boost::asio::read(socket, boost::asio::buffer(buf2), boost::asio::transfer_exactly(len));
      if (read > 0)
      {
        seq ^= buf2[0];
      }
      else
      {
        seq ^= 0xAB;
      }

      for (unsigned int i = 0; i < len / 4; ++i)
        out << buf2[i];
    }    
  }
  return 0;
}


