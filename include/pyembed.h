/*
 * pyembed.h
 *
 * Created on: September, 2015
 * Author: csoare
 */

#ifndef PYEMBED_H
#define PYEMBED_H

#include <boost/python.hpp>
#include <string>
#include <map>

class PyEmbed
{
public:
	/**
	 * Singleton initializer and destroyer
	 */
	static PyEmbed* getInstance();
	static void destroyInstance();

	/**
	 * Loads script from given path
	 * @param path path of the given script
	 */
	void loadScript(const std::string& path);
	/**
	 * Constructs error message
	 * @param type usage type of the script
	 * @param type name or place of usage of the script
	 * @param path path of the script
	 * @param fname name of the loaded python function
	 * @param argCount number of expected arguments of the loaded function (not checking if -1)
	 * @return returned boost python function object
	 */
	boost::python::object* loadScript(
		const std::string& path,
		const std::string& fname,
		int argCount = -1,
		const std::string& type = "",
		const std::string& name = "");
	/**
	 * Gets python object of requested function from the namespace
	 * @param name name of the python function
	 * @param checkCount number of expected arguments of loaded function (not checking if -1)
	 * @return function object
	 */
	boost::python::object getFunction(const std::string& name, int checkCount = -1);

	/**
	 * Gets the number of arguments of the function
	 * @param func function object
	 * @return int number of arguments
	 */
	static int argCount(const boost::python::object& func);
	/**
	 * Gets the current python error as a string
	 * @return python error string
	 */
	static std::string getError();
	/**
	 * Logs the python error to its logit module
	 */
	static std::string logError();
	/**
	 * Logs the python error given as a string to its logit module
	 * @param err error message
	 */
	static void logError(const std::string& err);

private:
	PyEmbed();
	~PyEmbed();

	/**
	 * Constructs error message
	 * @param type usage type of the script
	 * @param type name name or place of usage of the script
	 * @param path path of the script
	 * @param fname name of the loaded python function
	 * @return constructed intialization failure message
	 */
	static std::string getInitMsg(
		const std::string& type,
		const std::string& name,
		const std::string& path,
		const std::string& fname);

	static PyEmbed* m_instance;

	PyThreadState* m_pst;
	boost::python::object m_main, m_mainNamespace;

	std::map<std::string, bool> m_scripts;
	std::map<std::string, boost::python::object> m_functions;
};

class PyGilGuard
{
public:
	PyGilGuard();
	~PyGilGuard();

	/**
	 * Manually unlocks the guard
	 */
	void unlock();

private:
	PyGILState_STATE m_state;
	bool m_locked;
};

class PyThreadGuard
{
public:
	PyThreadGuard();
	~PyThreadGuard();


	/**
	 * Manually unlocks the guard
	 */
	void unlock();

private:
	PyThreadState* m_state;
};

#endif
