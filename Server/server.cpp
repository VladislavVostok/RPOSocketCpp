#include "cross_platform_sockets.h"
#include "product.h"
#include <thread>
#include <sstream>

using namespace std;

map<int, Product> productDatabase = {

	{ 1, {1, "Ноутбук Lenovo IdeaPad", 54999.99, 12, "Ноутбуки"} },
	{ 2, {2, "Смарфон Xiaomi Redmi Note", 24999.50, 34, "Смартфоны"} },
	{ 3, {3, "Наушники", 19999.99, 8, "Аксессуары"} },
	{ 4, {4, "Монитор LG 328y547", 42999.95, 7, "Мониторы"} },

};

string processRequest(const string& request) {
	if (request == "LIST_ALL") {
		if (productDatabase.empty()) {
			return "ERROR|DB is empty";
		}

		string response;

		for (const auto& [id, product] : productDatabase) {
			response += product.to_string_custom() + "\n";
		}
		return response;
	}
	else if (request.rfind("GET ", 0) == 0) {
		int productId;
		try {
			productId = stoi(request.substr(4));
		}
		catch (...) {
			return "ERROR|Неверный формат ID товара";
		}

		auto it = productDatabase.find(productId);
		if (it == productDatabase.end()) {
			return "ERROR|Товар с ID " + to_string(productId) + " не найден";
		}

		return "OK|" + it->second.to_string_custom();

	}

	return "ERROR|Неизвестная команда";

}

void handleClient(int clientSocket) {
	try {
		char buffer[1024] = { 0 };
		while (true) {
			int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
			check_socket_error(bytesRead, "Ошибка чтения данных");

			if (bytesRead <= 0) {
				return;
			}

			string request(buffer, bytesRead);
			cout << "Запрос получен " << request << endl;

			string response = processRequest(request);

			int byteSent = send(clientSocket, response.c_str(), response.size(), MSG_NOSIGNAL);
			check_socket_error(byteSent, "Данные клиенту не отправлены.");

			cout << "Отправлен ответ: " << response.size() << " байт" << endl;
		}

	}catch(const exception& e){
		cerr << "Ошибка обработки " << e.what() << endl;
	}
}

int main() {

	setlocale(LC_ALL, "");
	try {

		init_network();

		int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
		check_socket_error(serverSocket, "Creatin socket error!");

		sockaddr_in serverAddr;
		serverAddr.sin_family = AF_INET;
		//serverAddr.sin_addr.s_addr = INADDR_ANY;   // localhost 127.0.0.1
		inet_pton(AF_INET, "45.139.78.128", &serverAddr.sin_addr);
		serverAddr.sin_port = htons(5555);

		int opt = 1;

	#ifdef _WIN32
		check_socket_error(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)), "Installing error SO_REUSEDADDR");
	#else
		check_socket_error(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)), "Installing error SO_REUSEDADDR");
	#endif

		check_socket_error(bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)), "bindin socket error!");
		check_socket_error(listen(serverSocket, 10), "listening socket error!");

		cout << "Server started on 5555 port" << endl;

		while (true) {
			

			sockaddr_in clientAddr;
			socklen_t clientAddrLen = sizeof(clientAddr);
			int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrLen);
		
			if (clientSocket == INVALID_SOCKET) {
				cerr << "Ошибка принятия соединения" << endl;
				continue;
			}

			char clientIP[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
			cout << "New connect from " << clientIP << ":" << ntohs(clientAddr.sin_port) << endl;
			
			// Функция обработки клиентских запросов
			//std::thread t1([&]()
			//	{
			//		handleClient(clientSocket);
			//		closesocket(clientSocket);
			//	});

			handleClient(clientSocket);
			close_socket(clientSocket);

			cout << "Соединение с клиентом закрыто" << endl;

		}

		close_socket(serverSocket);
		cleanup_network();
	}
	catch (const exception& e) {
		cerr << "Critical error: " << e.what() << endl;
		cleanup_network();
		return 1;
	}

	return 0;
}