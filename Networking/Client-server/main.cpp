#pragma comment(lib, "Ws2_32.lib")


#include "Client.h"

int main() {
  Client client("54.83.193.50", 2323);
  client.Initialize();

  while (true) {
	unsigned int length;
    auto data = client.GetNextPacket(&length);
	;
  }

  return 0;
}