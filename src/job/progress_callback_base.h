#ifndef CODEX_WORKER_PROGRESS_CALLBACK_BASE_H
#define CODEX_WORKER_PROGRESS_CALLBACK_BASE_H


/**
 *
 */
class progress_callback_base
{
public:
	virtual ~progress_callback_base()
	{
	}

	virtual void job_started(std::string job_id) = 0;
	virtual void job_ended(std::string job_id) = 0;
	virtual void task_completed(std::string job_id, std::string task_id) = 0;
	virtual void task_failed(std::string job_id, std::string task_id) = 0;
};

class empty_progress_callback : public progress_callback_base
{
public:
	virtual ~empty_progress_callback()
	{
	}

	virtual void job_started(std::string job_id)
	{
	}
	virtual void job_ended(std::string job_id)
	{
	}
	virtual void task_completed(std::string job_id, std::string task_id)
	{
	}
	virtual void task_failed(std::string job_id, std::string task_id)
	{
	}
};


#endif // CODEX_WORKER_PROGRESS_CALLBACK_BASE_H
