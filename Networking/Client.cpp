#include "Client.h"

#include <string>
using namespace std;


Client::Client(const char * ip, size_t port) {
#ifdef WIN32
  if (::WSAStartup(MAKEWORD(2, 2), &m_connectionData) != 0)
    throw;
#endif

  m_socket = ::socket(
    AF_INET, /* use IPv4 address family */
    SOCK_STREAM, /* provides sequenced, reliable, two-way, connection-based byte
                    streams with an OOB data transmission mechanism. This socket
                    type TCP for the AF_INET */
    IPPROTO_TCP); /* using TCP as a protocol */

  if (m_socket == INVALID_SOCKET)
    throw;

  SOCKADDR_IN address;
  ::memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = ::inet_addr(ip);
  address.sin_port = htons(port);

  int result = 0;
  if ((result = connect(m_socket, (sockaddr*) &address, sizeof(address))) != 0)
      throw;
}


Client::~Client() {
  if (m_socket != INVALID_SOCKET)
    ::closesocket(m_socket);

#ifdef WIN32
  if (::WSACleanup() != 0)
    throw;
#endif
}

void Client::Initialize() {
  vector<char> buffer; // stores in UTF-8, wstring - UTF-16
  auto result = Recieve(buffer, '\n');
  cout << "recv " << result << endl;

  string str(buffer.begin(), buffer.end());
  auto start = str.find(":");
  auto end = str.find('\n');
  str = str.substr(start + 1, end - start - 1);
  cout << "Challenge:" << str<< endl;

  str = "IAM:" + str + ":" + "karapish@gmail.com:at\n";
  buffer.assign(str.begin(), str.end());
  result = Send(buffer);

  result = Recieve(buffer, '\n');
  cout << "recv " << result << endl;

  str.assign(buffer.begin(), buffer.end());
  start = str.find(":");
  end = str.find('\n');
  str = str.substr(start + 1, end - start - 1);
  cout << "Code:" << str << endl;
}

int Client::Recieve(vector<char>& buffer, char until) {
  const int size = 1;
  char result[size] = { 0 };
  buffer.clear();
  int count = 0;

  while (true) {
	int value = ::recv(
		m_socket,
		result,
		size,
		0);

	int last_error = ::WSAGetLastError();
	if (last_error != 0)
		std::cout << "WSAGetLastError() ~ " << last_error << std::endl;
	
	buffer.push_back(*result);
	count++;

	if (*result == until) break;
  }

  return count;
}

int Client::Send(const vector<char>& buffer) const {
  return ::send(
	  m_socket,
	  &buffer[0],
	  buffer.size(),
	  0);
}

vector<char> Client::GetNextPacket(unsigned int * length) {
  vector<char> seqData;
  GetNextHeaderItem(seqData);
  unsigned int seq = ToLittleEndian(seqData);
  unsigned int seqBig = ToBigEndian(seqData);

  vector<char> chkData;
  GetNextHeaderItem(chkData);
  unsigned int chk = ToLittleEndian(chkData);

  vector<char> lenData;
  GetNextHeaderItem(lenData);
  *length = ToLittleEndian(lenData);
  
  cout << "seq " << seq << "(" << seqBig << ")  chk " << chk << "  len " << *length;

  vector<char> buffer;
  buffer.resize(*length);

  int value = ::recv(
	  m_socket,
	  &buffer[0],
	  *length,
	  0);
  
  int newLength = *length;

  while (newLength % 4 != 0)
	  newLength++;

  vector<char> buffer2(buffer);

  buffer2.resize(newLength);
  for (int i = *length; i < newLength; ++i) {
	  buffer2[i] = static_cast<byte>(0xAB);
  }

  unsigned int xor = 0;
  for (int i = 0; i < newLength - 4; i=i+4) {
	  vector<char> buf(buffer2.begin() + i, buffer2.begin() + i + 4);
	  //auto dataBig = ToBigEndian(buf);
	  auto dataLit = ToLittleEndian(buf);
	  xor = xor ^ seqBig ^ dataLit;
  }

  unsigned int xorLit =
	  ((xor >> 24) & 0xff) | // move byte 3 to byte 0
	  ((xor << 8) & 0xff0000) | // move byte 1 to byte 2
	  ((xor >> 8) & 0xff00) | // move byte 2 to byte 1
	  ((xor << 24) & 0xff000000);// byte 0 to byte 3

  cout << "  xor " << xor << "(" << xorLit << ")" << endl;

  return buffer;
}
