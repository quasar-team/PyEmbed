/*
 * pyembed.h
 *
 * Created on: September, 2015
 * Author: Cristian-Valeriu Soare
 * E-mail: soare.cristian21@gmail.com
 */

#include <pyembed.h>

#include <LogIt.h>
#include <LogItComponentIDs.h>
#include <iostream>
#include <sstream>

using namespace std;
namespace py = boost::python;

PyEmbed* PyEmbed::m_instance;

PyEmbed* PyEmbed::getInstance()
{
	if (!m_instance)
		m_instance = new PyEmbed();
	return m_instance;
}

void PyEmbed::destroyInstance()
{
	delete m_instance;
	m_instance = 0;
}

PyEmbed::PyEmbed()
{
	Py_Initialize();
    PyEval_InitThreads();

    m_main = py::import("__main__");
    m_mainNamespace = m_main.attr("__dict__");
    m_pst = PyEval_SaveThread();
}

PyEmbed::~PyEmbed()
{
	PyEval_RestoreThread(m_pst);
    Py_Finalize();
}

void PyEmbed::loadScript(const string& path)
{
	if (m_scripts[path])
		return;
	m_scripts[path] = true;
	try {
		py::exec_file(path.c_str(), m_mainNamespace);
	} catch (const py::error_already_set&) {
		throw runtime_error(getError().c_str());
	}

	py::dict d = py::extract<py::dict>(m_mainNamespace);
	py::list keys = d.keys();
	py::list values = d.values();

	// check if the function name (key) has been already loaded, but its pointer has changed
	for (unsigned int i = 0; i < py::len(keys); i++) {
		string key(py::extract<const char*>(keys[i]));
		if (m_functions.find(key) != m_functions.end() && m_functions[key].ptr() != static_cast<py::object>(values[i]).ptr())
			throw runtime_error(key + " redeclared in " + path);
		m_functions[key] = values[i];
	}
}

py::object* PyEmbed::loadScript(
	const string& path,
	const string& fname,
	int argCount,
	const string& type,
	const string& name)
{
	std::string errmsg = getInitMsg(type, name, path, fname);
	try {
		loadScript(path);
		return new boost::python::object(getFunction(fname, argCount));
	} catch (const std::exception& e) {
		errmsg += "\n" + std::string(e.what());
		throw std::runtime_error(errmsg.c_str());
	}
}

py::object PyEmbed::getFunction(const string& name, int checkCount)
{
	py::object func;
	try {
		func = m_main.attr(name.c_str());
	} catch (py::error_already_set&) {
		throw runtime_error(getError().c_str());
	}
	if (checkCount < 0 || argCount(func) == checkCount)
		return func;

	ostringstream oss;
	oss << "Ivalid number of parameters for function " << name << ": given " << argCount(func) << " expected " << checkCount;
	throw runtime_error(oss.str().c_str());
}

int PyEmbed::argCount(const py::object& func)
{
	return py::extract<size_t>(func.attr("func_code").attr("co_argcount"));
}

string PyEmbed::logError()
{
	string err = getError();
	if (!err.empty())
		LOG(Log::ERR, LogItComponentID::PyEmbed) << err;
	return err;
}

void PyEmbed::logError(const string& err)
{
	LOG(Log::ERR, LogItComponentID::PyEmbed) << err;
}

string PyEmbed::getError()
{
	if (!PyErr_Occurred())
		return string();

    PyObject *exc,*val,*tb;
    py::object formatted_list, formatted;
    PyErr_Fetch(&exc,&val,&tb);
    py::handle<> hexc(exc),hval(py::allow_null(val)),htb(py::allow_null(tb));
    py::object traceback(py::import("traceback"));
    if (!tb) {
        py::object format_exception_only(traceback.attr("format_exception_only"));
        formatted_list = format_exception_only(hexc,hval);
    } else {
        py::object format_exception(traceback.attr("format_exception"));
        formatted_list = format_exception(hexc,hval,htb);
    }
    formatted = py::str("\n").join(formatted_list);
    string logText((string)py::extract<string>(formatted));
    auto iter = logText.end()-1;
    if (*iter == '\n')
    	*iter = ' ';

    py::handle_exception();
    PyErr_Clear();
    return logText;
}

std::string PyEmbed::getInitMsg(
	const std::string& type,
	const std::string& name,
	const std::string& path,
	const std::string& fname)
{
	std::string msg("Problem initializing " + type + " script for " + name + "\n");
	msg += "script: " + path + "\n";
	msg += "function: " + fname;
	return msg;
}

PyGilGuard::PyGilGuard()
	: m_state(PyGILState_Ensure())
	, m_locked(true) {}

PyGilGuard::~PyGilGuard()
{
	unlock();
}

void PyGilGuard::unlock()
{
	if (m_locked) {
		PyGILState_Release(m_state);
		m_locked = false;
	}
}

PyThreadGuard::PyThreadGuard()
	: m_state(PyEval_SaveThread()) {}

PyThreadGuard::~PyThreadGuard()
{
	unlock();
}

void PyThreadGuard::unlock()
{
	if (m_state) {
		PyEval_RestoreThread(m_state);
		m_state = NULL;
	}
}
