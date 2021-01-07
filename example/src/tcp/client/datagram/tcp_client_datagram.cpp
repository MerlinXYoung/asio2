// When compiling with vs under linux, you need to copy the "asio,beast,ceral,fmt" folders to
// the /usr/local/include directory first, and copy the "libcrypto.a,libssl.a" files to 
// /usr/local/lib directory first. "libcrypto.a,libssl.a" is in "asio2/lib/x64".
#include <cstdio>
#include <cassert>
#include <cstring>
#include <asio2/tcp/tcp_client.hpp>
enum match_role_type{
	DGRAM,
	FIXED2,
	FIXED4,
};
int main(int argc , char** argv)
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	match_role_type type{DGRAM};
	if(argc > 1){
		if(strcmp(argv[1], "dgram") ==0 )
		{

		}
		else if(strcmp(argv[1], "fixed2") ==0 ){
			type = FIXED2;
		}
		else if(strcmp(argv[1], "fixed4") ==0 ){
			type = FIXED4;
		}
		else{
			fprintf(stderr, "fuck match role type\n");
			exit(1);
		}
	}

	std::string_view host = "127.0.0.1";
	std::string_view port = "8027";

	asio2::tcp_client client;

	client.bind_connect([&](asio::error_code ec)
	{
		if (asio2::get_last_error())
			printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());

		std::string s;
		s += '#';
		s += char(1);
		s += 'a';

		// Beacuse the server specify the "max recv buffer size" to 1024, so if we
		// send a too long packet, then this client will be disconnect .
		//s.resize(1500);

		client.send(s);

	}).bind_disconnect([](asio::error_code ec)
	{
		printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_recv([&](std::string_view sv)
	{
		printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

		std::string s;
		s += '#';
		uint8_t len = uint8_t(100 + (std::rand() % 100));
		s += char(len);
		for (uint8_t i = 0; i < len; i++)
		{
			s += (char)((std::rand() % 26) + 'a');
		}

		client.send(std::move(s));

	});


	switch(type){
	case DGRAM:
		client.start(host, port, asio2::use_dgram); // dgram tcp
		break;
	case FIXED2: 
		client.start(host, port, asio2::use_fixed2); // fixed2 tcp
		break;
	case FIXED4: 
		client.start(host, port, asio2::use_fixed4); // fixed4 tcp
		break;
	default:
		assert(false);
	}

	while (std::getchar() != '\n');

	return 0;
}
