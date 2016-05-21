#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <thread>
#include <zmq.hpp>

#include "../src/connection_proxy.h"
#include "../src/job/progress_callback.h"

using namespace testing;


TEST(progress_callback, basic)
{
	// needed initialization
	auto context = std::make_shared<zmq::context_t>(1);
	zmq::socket_t socket(*context, ZMQ_PAIR);
	socket.bind("inproc://" + PROGRESS_SOCKET_ID);

	std::string command = "progress";
	std::string job_id = "test_job_id";
	std::string task_id = "test_task_id";

	std::thread r([&]() {
		progress_callback callback(context, nullptr);
		callback.submission_downloaded(job_id);
		callback.job_started(job_id);
		callback.task_completed(job_id, task_id);
		callback.task_failed(job_id, task_id);
		callback.job_ended(job_id);
		callback.job_results_uploaded(job_id);
	});

	std::vector<std::string> result;
	bool terminate;

	// receive message submission downloaded
	std::vector<std::string> expected = {command, job_id, "DOWNLOADED"};
	helpers::recv_from_socket(socket, result, &terminate);
	ASSERT_EQ(result, expected);

	// receive message job started
	expected = {command, job_id, "STARTED"};
	helpers::recv_from_socket(socket, result, &terminate);
	ASSERT_EQ(result, expected);

	// receive message task completed
	expected = {command, job_id, "TASK", task_id, "COMPLETED"};
	helpers::recv_from_socket(socket, result, &terminate);
	ASSERT_EQ(result, expected);

	// receive message task failed
	expected = {command, job_id, "TASK", task_id, "FAILED"};
	helpers::recv_from_socket(socket, result, &terminate);
	ASSERT_EQ(result, expected);

	// receive message job ended
	expected = {command, job_id, "ENDED"};
	helpers::recv_from_socket(socket, result, &terminate);
	ASSERT_EQ(result, expected);

	// receive message results uploaded
	expected = {command, job_id, "UPLOADED"};
	helpers::recv_from_socket(socket, result, &terminate);
	ASSERT_EQ(result, expected);

	// end all necessary things
	context->close();
	r.join();
}
