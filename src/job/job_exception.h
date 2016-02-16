#ifndef CODEX_WORKER_JOB_EXCEPTION_H
#define CODEX_WORKER_JOB_EXCEPTION_H

/**
 * Job exception class.
 */
class job_exception : public std::exception
{
public:
	/**
	 * Generic job exception with no specification.
	 */
	job_exception() : what_("Generic job exception")
	{
	}
	/**
	 * Exception with some brief description.
	 * @param what textual description of a problem
	 */
	job_exception(const std::string &what) : what_(what)
	{
	}
	/**
	 * Virtual destructor.
	 */
	virtual ~job_exception()
	{
	}
	/**
	 * Return description of this exception.
	 * @return C string
	 */
	virtual const char *what() const noexcept
	{
		return what_.c_str();
	}

protected:
	std::string what_;
};

#endif // CODEX_WORKER_JOB_EXCEPTION_H
