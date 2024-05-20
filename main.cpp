#include <mqtt/async_client.h>

#include <iostream>
#include <atomic>
#include <thread>

constexpr const char* SERVER_ADDRESS{ "tcp://192.168.7.1:1883" };
constexpr const char* CLIENT_ID{ "RandomLogDataPublisher" };
constexpr const char* TOPIC{ "TestTopic" };
constexpr const char* PERSIST_DIR {"/tmp/mqtt-persist"};
constexpr const char* LWT_PAYLOAD = "This is my last will";

/**
 * A callback class for use with the main MQTT client.
 */
class callback : public virtual mqtt::callback
{
public:
	void connection_lost(const std::string& cause) override {
		std::cout << "\nConnection lost" << std::endl;
		if (!cause.empty())
			std::cout << "\tcause: " << cause << std::endl;
	}

	void delivery_complete(mqtt::delivery_token_ptr tok) override {
		std::cout << "\tDelivery complete for token: "
			<< (tok ? tok->get_message_id() : -1) << std::endl;
	}
};

/////////////////////////////////////////////////////////////////////////////

/**
 * A base action listener.
 */
class action_listener : public virtual mqtt::iaction_listener
{
protected:
	void on_failure(const mqtt::token& tok) override {
		std::cout << "\tListener failure for token: "
			<< tok.get_message_id() << std::endl;
	}

	void on_success(const mqtt::token& tok) override {
		std::cout << "\tListener success for token: "
			<< tok.get_message_id() << std::endl;
	}
};

/////////////////////////////////////////////////////////////////////////////

/**
 * A derived action listener for publish events.
 */
class delivery_action_listener : public action_listener
{
	std::atomic<bool> done_;

	void on_failure(const mqtt::token& tok) override {
		action_listener::on_failure(tok);
		done_ = true;
	}

	void on_success(const mqtt::token& tok) override {
		action_listener::on_success(tok);
		done_ = true;
	}

public:
	delivery_action_listener() : done_(false) {}
	[[nodiscard]] bool is_done() const { return done_; }
};


int main() {

    const int QOS = 2;
	mqtt::async_client client(SERVER_ADDRESS, CLIENT_ID, PERSIST_DIR);
	callback cb;
	client.set_callback(cb);
	auto connOpts = mqtt::connect_options_builder()
		.clean_session()
	    .automatic_reconnect(true)
		.will(mqtt::message(TOPIC, LWT_PAYLOAD, strlen(LWT_PAYLOAD), QOS, false))
		.finalize();
	try {
		std::cout << "\nConnecting..." << std::endl;
		mqtt::token_ptr conntok = client.connect(connOpts);
		std::cout << "Waiting for the connection..." << std::endl;
		conntok->wait();
		std::cout << "  ...OK" << std::endl;

		// First use a message pointer.
		for (int i=0;i < 100000;++i) {
			std::cout << "Publishing message no " << i << std::endl;
			std::string msg = "Sending message " + std::to_string(i);
			auto pubmsg = mqtt::make_message(TOPIC,msg.c_str(),msg.length(),QOS,true);
			client.publish(pubmsg)->wait();
			std::cout << " ....OK" <<std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

		}


		// Double check that there are no pending tokens

		// Disconnect
		std::cout << "\nDisconnecting..." << std::endl;
		client.disconnect()->wait();
		std::cout << "  ...OK" << std::endl;
	}
	catch (const mqtt::exception& exc) {
		std::cerr << exc.what() << std::endl;
		return 1;
	}

    return 0;
}
