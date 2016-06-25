#ifndef RECODEX_WORKER_PROGRESS_CALLBACK_BASE_H
#define RECODEX_WORKER_PROGRESS_CALLBACK_BASE_H

/**
 * Callback which is used in @ref job_evaluator and @ref job itself to indicate its state.
 * Can be used to inform user about evaluating particular submissions/jobs.
 * @note All methods should be exceptionless, otherwise behavior is undefined.
 */
class progress_callback_base
{
public:
	/**
	 * Stated for completion and for derived classes.
	 */
	virtual ~progress_callback_base()
	{
	}

	/**
	 * Indicates that submission was successfully downloaded from fileserver.
	 * @param job_id unique identification of downloaded job
	 * @note Implementation should not throw an exception.
	 */
	virtual void submission_downloaded(const std::string &job_id) = 0;
	/**
	 * After calling this, results should be visible for end users.
	 * @param job_id unique identification of job which results were uploaded
	 * @note Implementation should not throw an exception.
	 */
	virtual void job_results_uploaded(const std::string &job_id) = 0;
	/**
	 * Indicates job was started and all execution machinery was setup and is ready to roll.
	 * @param job_id unique identification of soon to be evaluated job
	 * @note Implementation should not throw an exception.
	 */
	virtual void job_started(const std::string &job_id) = 0;
	/**
	 * Calling this function should indicate that all was evaluated, just results have to be bubble through.
	 * @param job_id unique identifier of executed job
	 * @note Implementation should not throw an exception.
	 */
	virtual void job_ended(const std::string &job_id) = 0;
	/**
	 * Tells that task with given particular ID was just successfully completed.
	 * @param job_id unique identification of job
	 * @param task_id unique identification of successfully completed task
	 * @note Implementation should not throw an exception.
	 */
	virtual void task_completed(const std::string &job_id, const std::string &task_id) = 0;
	/**
	 * Indicates that task with given ID failed in execution.
	 * Information whether task was esential (fatal-failure) is not given.
	 * This should be detected through job_ended callback.
	 * @param job_id unique identification of job
	 * @param task_id unique identification of failed task
	 * @note Implementation should not throw an exception.
	 */
	virtual void task_failed(const std::string &job_id, const std::string &task_id) = 0;
	/**
	 * Parental task of given one failed and this one was skipped.
	 * @param job_id unique identification of job
	 * @param task_id unique identification of skipped task
	 * @note Implementation should not throw an exception.
	 */
	virtual void task_skipped(const std::string &job_id, const std::string &task_id) = 0;
};

/**
 * Implementation of @ref progress_callback_base which has all functions empty.
 * Its provided because of better looking code through calling this class instead of nullptr checking.
 * @note See @ref progress_callback_base for better description of this class and its methods.
 */
class empty_progress_callback : public progress_callback_base
{
public:
	/**
	 * Stated for completion.
	 */
	virtual ~empty_progress_callback()
	{
	}

	/**
	 * Empty indication.
	 * @param job_id
	 */
	virtual void submission_downloaded(const std::string &job_id)
	{
	}
	/**
	 * Empty indication.
	 * @param job_id
	 */
	virtual void job_results_uploaded(const std::string &job_id)
	{
	}
	/**
	 * Empty indication.
	 * @param job_id
	 */
	virtual void job_started(const std::string &job_id)
	{
	}
	/**
	 * Empty indication.
	 * @param job_id
	 */
	virtual void job_ended(const std::string &job_id)
	{
	}
	/**
	 * Empty indication.
	 * @param job_id
	 * @param task_id
	 */
	virtual void task_completed(const std::string &job_id, const std::string &task_id)
	{
	}
	/**
	 * Empty indication.
	 * @param job_id
	 * @param task_id
	 */
	virtual void task_failed(const std::string &job_id, const std::string &task_id)
	{
	}
	/**
	 * Empty indication.
	 * @param job_id
	 * @param task_id
	 */
	virtual void task_skipped(const std::string &job_id, const std::string &task_id)
	{
	}
};

#endif // RECODEX_WORKER_PROGRESS_CALLBACK_BASE_H
