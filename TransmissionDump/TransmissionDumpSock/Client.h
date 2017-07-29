#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning( disable : 4996 ) 
#include <winsock2.h>
#include <string>
#include <iostream>
#include <vector>

using namespace std;

class Client {
private:
  WSADATA m_connectionData;
  SOCKET m_socket = INVALID_SOCKET;

  int Recieve(vector<char>& buffer, char until);
  int Send(const vector<char>& buffer) const;

  int GetNextHeaderItem(vector<char>& input) {
	  input.clear();
	  input.resize(4);

	  int value = ::recv(
		  m_socket,
		  &input[0],
		  4,
		  0);

	  return value;
  }

  unsigned int ToLittleEndian(vector<char> data)
  {
	  return ((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]);
  }

  unsigned int ToBigEndian(vector<char> data)
  {
	  return ((data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0]);
  }

public:
  Client(const char* ip, size_t port);
  virtual ~Client();

  void Initialize();
  vector<char> GetNextPacket(unsigned int * length);
};

