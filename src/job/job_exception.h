#ifndef RECODEX_WORKER_JOB_EXCEPTION_H
#define RECODEX_WORKER_JOB_EXCEPTION_H

#include <exception>
#include <string>

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
	~job_exception() override = default;

	/**
	 * Return description of this exception.
	 * @return C string
	 */
	const char *what() const noexcept override
	{
		return what_.c_str();
	}

protected:
	/** A textual description of the exception */
	std::string what_;
};


/**
 * Special exception for unrecoverable errors in job execution.
 */
class job_unrecoverable_exception : public job_exception
{
public:
	/**
	 * Exception with description.
	 * @param what textual description
	 */
	job_unrecoverable_exception(const std::string &what) : job_exception(what)
	{
	}

	/** Destructor */
	~job_unrecoverable_exception() override = default;
};

#endif // RECODEX_WORKER_JOB_EXCEPTION_H
