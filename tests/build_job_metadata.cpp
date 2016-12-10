#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/helpers/config.h"

using namespace testing;
using namespace std;

TEST(job_metadata, build_all_from_yaml)
{
	auto job_yaml = YAML::Load("---\n"
							   "submission:\n"
							   "    job-id: eval5\n"
							   "    file-collector: localhost\n"
							   "    log: false\n"
							   "    hw-groups:\n"
							   "        - group1\n"
							   "tasks:\n"
							   "    - task-id: eval\n"
							   "      priority: 4\n"
							   "      fatal-failure: false\n"
							   "      dependencies:\n"
							   "          - fetch_test_1\n"
							   "          - fetch_test_2\n"
							   "      cmd:\n"
							   "          bin: recodex\n"
							   "          args:\n"
							   "              - -v\n"
							   "              - \"-f 01.in\"\n"
							   "      sandbox:\n"
							   "          name: fake\n"
							   "          stdin: before_stdin_${WORKER_ID}_after_stdin\n"
							   "          stdout: before_stdout_${JOB_ID}_after_stdout\n"
							   "          stderr: before_stderr_${RESULT_DIR}_after_stderr\n"
							   "          limits:\n"
							   "              - hw-group-id: group1\n"
							   "                time: 5\n"
							   "                wall-time: 6\n"
							   "                extra-time: 7\n"
							   "                stack-size: 50000\n"
							   "                memory: 60000\n"
							   "                parallel: 1\n"
							   "                disk-size: 50\n"
							   "                disk-files: 10\n"
							   "                chdir: ${EVAL_DIR}\n"
							   "                environ-variable:\n"
							   "                    ISOLATE_TMP: /tmp\n"
							   "                bound-directories:\n"
							   "                    - src: path1/dir1\n"
							   "                      dst: path2/dir2\n"
							   "                      mode: RW\n"
							   "...\n");
	auto job_meta = helpers::build_job_metadata(job_yaml);

	EXPECT_EQ(job_meta->job_id, "eval5");
	EXPECT_EQ(job_meta->file_server_url, "localhost");
	EXPECT_EQ(job_meta->log, false);
	EXPECT_EQ(job_meta->hwgroups.size(), 1u);
	EXPECT_EQ(job_meta->hwgroups.at(0), "group1");
	EXPECT_EQ(job_meta->tasks.size(), 1u);

	auto metadata = job_meta->tasks[0];
	EXPECT_EQ(metadata->task_id, "eval");
	EXPECT_EQ(metadata->priority, 4u);
	EXPECT_EQ(metadata->fatal_failure, false);
	auto deps = std::vector<std::string>{"fetch_test_1", "fetch_test_2"};
	EXPECT_EQ(metadata->dependencies, deps);
	EXPECT_EQ(metadata->binary, "recodex");
	auto args = std::vector<std::string>{"-v", "-f 01.in"};
	EXPECT_EQ(metadata->cmd_args, args);
	EXPECT_EQ(metadata->type, task_type::INNER);

	auto sandbox = metadata->sandbox;
	EXPECT_NE(sandbox, nullptr);
	EXPECT_EQ(sandbox->name, "fake");

	EXPECT_EQ(sandbox->loaded_limits.size(), 1u);
	std::shared_ptr<sandbox_limits> limits = sandbox->loaded_limits.at("group1");
	EXPECT_NE(limits, nullptr);
	EXPECT_EQ(limits->cpu_time, 5);
	EXPECT_EQ(limits->wall_time, 6);
	EXPECT_EQ(limits->extra_time, 7);
	EXPECT_EQ(limits->stack_size, 50000u);
	EXPECT_EQ(limits->memory_usage, 60000u);
	EXPECT_EQ(limits->processes, 1u);
	EXPECT_EQ(limits->disk_size, 50u);
	EXPECT_EQ(limits->disk_files, 10u);
	EXPECT_EQ(limits->chdir, "${EVAL_DIR}");
	EXPECT_EQ(limits->std_input, "before_stdin_${WORKER_ID}_after_stdin");
	EXPECT_EQ(limits->std_output, "before_stdout_${JOB_ID}_after_stdout");
	EXPECT_EQ(limits->std_error, "before_stderr_${RESULT_DIR}_after_stderr");

	EXPECT_EQ(limits->environ_vars.size(), 1u);
	auto envs = std::pair<std::string, std::string>{"ISOLATE_TMP", "/tmp"};
	EXPECT_EQ(limits->environ_vars[0], envs);

	EXPECT_EQ(limits->bound_dirs.size(), 1u);
	auto dirs = std::tuple<std::string, std::string, sandbox_limits::dir_perm>{
		"path1/dir1", "path2/dir2", sandbox_limits::dir_perm::RW};
	EXPECT_EQ(limits->bound_dirs[0], dirs);
}

TEST(job_metadata, queue_of_tasks)
{
	auto job_yaml = YAML::Load("---\n"
							   "submission:\n"
							   "    job-id: 5\n"
							   "    file-collector: localhost\n"
							   "    hw-groups:\n"
							   "        - group1\n"
							   "tasks:\n"
							   "    - task-id: A\n"
							   "      type: InNeR\n"
							   "      priority: 1\n"
							   "      fatal-failure: false\n"
							   "      cmd:\n"
							   "          bin: mkdir\n"
							   "          args:\n"
							   "              - hello\n"
							   "    - task-id: B\n"
							   "      type: InItIaTiOn\n"
							   "      priority: 4\n"
							   "      fatal-failure: true\n"
							   "      dependencies:\n"
							   "          - A\n"
							   "      cmd:\n"
							   "          bin: mkdir\n"
							   "          args:\n"
							   "              - hello\n"
							   "    - task-id: C\n"
							   "      type: ExEcUtIoN\n"
							   "      priority: 6\n"
							   "      fatal-failure: false\n"
							   "      dependencies:\n"
							   "          - B\n"
							   "          - D\n"
							   "      cmd:\n"
							   "          bin: mkdir\n"
							   "          args:\n"
							   "              - hello\n"
							   "    - task-id: D\n"
							   "      type: EvAlUaTiOn\n"
							   "      priority: 8\n"
							   "      fatal-failure: false\n"
							   "      dependencies:\n"
							   "          - B\n"
							   "          - D\n"
							   "      cmd:\n"
							   "          bin: cp\n"
							   "          args:\n"
							   "              - hello\n"
							   "...\n");
	auto job_meta = helpers::build_job_metadata(job_yaml);

	auto tasks = job_meta->tasks;
	EXPECT_EQ(tasks.size(), 4u);
	EXPECT_EQ(tasks[0]->task_id, "A");
	EXPECT_EQ(tasks[0]->priority, 1u);
	EXPECT_EQ(tasks[0]->fatal_failure, false);
	EXPECT_EQ(tasks[0]->binary, "mkdir");
	auto args0 = std::vector<std::string>{"hello"};
	EXPECT_EQ(tasks[0]->cmd_args, args0);
	EXPECT_EQ(tasks[0]->type, task_type::INNER);

	EXPECT_EQ(tasks[1]->task_id, "B");
	EXPECT_EQ(tasks[1]->priority, 4u);
	EXPECT_EQ(tasks[1]->fatal_failure, true);
	auto deps1 = std::vector<std::string>{"A"};
	EXPECT_EQ(tasks[1]->dependencies, deps1);
	EXPECT_EQ(tasks[1]->binary, "mkdir");
	auto args1 = std::vector<std::string>{"hello"};
	EXPECT_EQ(tasks[1]->cmd_args, args1);
	EXPECT_EQ(tasks[1]->type, task_type::INITIATION);

	EXPECT_EQ(tasks[2]->task_id, "C");
	EXPECT_EQ(tasks[2]->priority, 6u);
	EXPECT_EQ(tasks[2]->fatal_failure, false);
	auto deps2 = std::vector<std::string>{"B", "D"};
	EXPECT_EQ(tasks[2]->dependencies, deps2);
	EXPECT_EQ(tasks[2]->binary, "mkdir");
	auto args2 = std::vector<std::string>{"hello"};
	EXPECT_EQ(tasks[2]->cmd_args, args2);
	EXPECT_EQ(tasks[2]->type, task_type::EXECUTION);

	EXPECT_EQ(tasks[3]->task_id, "D");
	EXPECT_EQ(tasks[3]->priority, 8u);
	EXPECT_EQ(tasks[3]->binary, "cp");
	EXPECT_EQ(tasks[3]->type, task_type::EVALUATION);
}
