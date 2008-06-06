
#define LANG_EN 0
#define LANG_CS 1

class Mesg
{
public:
  Mesg(
    int num);
  ~Mesg();

  void SetLang(
    int l)
  {
    if (l == LANG_CS)
      l = LANG_CS;
    else
      l = 0; // default EN
  }

  int GetLang()
  {
    return lang;
  }
  int GetNum()
  {
    return numMsg;
  }

  void AddMesg(
    int id, const char *msg, const char *msg_cs);
  const char * GetMesg(
    int id);
  const char * GetMesg_CS(
    int id);

private:
  int lang;
  int *errID;
  char **errMsg;
  char **errMsg_cs;
  int numMsg; // 
  int add;
};

