#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <Windows.h>

using namespace std;

class Printable
{
private:
    static Printable * s_pInstance;
    static int s_counter;

    CRITICAL_SECTION m_lock;
    ofstream m_file;
    string m_name;
    SYSTEMTIME m_lt;

    Printable(string& name) {
        string s = CreateLogName(name);
        InitializeCriticalSection(&m_lock);
        m_file.open(s);
        m_name = s;
    }

    virtual ~Printable() {
        m_file.close();
        DeleteCriticalSection(&m_lock);
    }

    string CreateLogName(const string& name)
    {
        ostringstream convert;
        convert << ++s_counter;
        string counter_text = convert.str();
        string ext = ".txt";
        return string(name + counter_text + ext);
    }

public:
   void print( const string& msg ) {
       EnterCriticalSection( &m_lock );
       GetLocalTime( &m_lt );
       m_file << "[" << m_lt.wMinute << ":" << m_lt.wSecond << "." << m_lt.wMilliseconds
              << ", th#" << GetCurrentThreadId() << "] " << msg << endl;
       LeaveCriticalSection( &m_lock );
   }

    void print( const ostringstream& msg ) {
        print(msg.str());
    }

    void print( const string& msg, int indentation)
    {
        std::string indent;
        for(int i = 0; i < indentation; ++i) {
            indent.append("...");
        }
        print(indent.append(msg));
    }

    static Printable& Create() {
        if (!s_pInstance) {
            s_pInstance = new Printable(string("INVESTIGATION"));
        }
        return *s_pInstance;
    }

    static void print( const string& msg, const string& prefix, int indentation) {
        Create().print(prefix + msg, indentation);
    }
};