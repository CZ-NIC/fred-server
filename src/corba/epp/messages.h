#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#include <iostream>
#include <vector>

#define LANG_EN 0
#define LANG_CS 1

class Mesg
{
public:
  Mesg();
  ~Mesg();

  void SetLang(
    int l)
  {
    if (l == LANG_CS)
      m_lang = LANG_CS;
    else
      m_lang = LANG_EN; // default EN
  }

  int GetLang()
  {
    return m_lang;
  }
  int GetNum()
  {
    return m_errID.size();
  }

  void AddMesg(const int id, const char *msg, const char *msg_cs);
  void AddMesg(const int id, const std::string msg, const std::string msg_cs);
  std::string GetMesg(int id);
  std::string GetMesg_CS(int id);

private:
  std::vector<std::string> m_errMsg;
  std::vector<std::string> m_errMsg_cs;
  std::vector<int> m_errID;
  int m_lang;
};

#endif // _MESSAGES_H_
