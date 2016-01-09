#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <zmq.hpp>
#include <thread>
#include <chrono>

#include "../src/job_receiver.h"
#include "../src/eval_request.h"
#include "../src/connection_proxy.h"

using namespace testing;

class mock_job_evaluator : public job_evaluator {
public:
	mock_job_evaluator () :
		job_evaluator(nullptr, nullptr, nullptr)
	{
	}

	MOCK_METHOD1(evaluate, eval_response(eval_request));
};

TEST(job_receiver, basic) {
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_PAIR);
	socket.bind("inproc://" JOB_SOCKET_ID);

	auto evaluator = std::make_shared<StrictMock<mock_job_evaluator>>();

	const char *id = "eval_123";
	const char *job_url = "http://dot.com/a.tar.gz";
	const char *result_url = "http://dot.com/results/123";

	eval_response response(id, "OK");

	EXPECT_CALL(*evaluator, evaluate(AllOf(
		Field(&eval_request::job_id, StrEq(id)),
		Field(&eval_request::job_url, StrEq(job_url)),
		Field(&eval_request::result_url, StrEq(result_url))
	))).Times(1).WillOnce(Return(response));

	job_receiver receiver(context, evaluator);
	std::thread r([&receiver] () {
		receiver.start_receiving();
	});

	socket.send("eval", 4, ZMQ_SNDMORE);
	socket.send(id, 8, ZMQ_SNDMORE);
	socket.send(job_url, 23, ZMQ_SNDMORE);
	socket.send(result_url, 26, 0);

	std::this_thread::sleep_for(std::chrono::milliseconds(1));

	zmq::message_t msg;
	bool retval;

	retval = socket.recv(&msg, ZMQ_NOBLOCK);
	ASSERT_TRUE(retval);
	ASSERT_EQ("eval_finished", std::string((char *) msg.data(), msg.size()));
	ASSERT_TRUE(msg.more());

	retval = socket.recv(&msg, ZMQ_NOBLOCK);
	ASSERT_TRUE(retval);
	ASSERT_EQ(id, std::string((char *) msg.data(), msg.size()));
	ASSERT_TRUE(msg.more());

	retval = socket.recv(&msg, ZMQ_NOBLOCK);
	ASSERT_TRUE(retval);
	ASSERT_EQ("OK", std::string((char *) msg.data(), msg.size()));
	ASSERT_FALSE(msg.more());

	context.close();
	r.join();
}

TEST(job_receiver, incomplete_msg) {
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_PAIR);
	socket.bind("inproc://" JOB_SOCKET_ID);

	// We don't expect any calls
	auto evaluator = std::make_shared<StrictMock<mock_job_evaluator>>();

	job_receiver receiver(context, evaluator);
	std::thread r([&receiver] () {
		receiver.start_receiving();
	});

	socket.send("eval", 4, ZMQ_SNDMORE);
	socket.send("foo", 3, ZMQ_SNDMORE);
	socket.send("foo", 3, 0);

	std::this_thread::sleep_for(std::chrono::milliseconds(1));
	context.close();
	r.join();
}
