#ifndef _GAYSTRING_H_
#define _GAYSTRING_H_


class GayString
{
public:
  GayString(const char *initial=NULL);
  ~GayString();
  void Set(const char *value);
  char *Get();

  void Append(const char *append);
  void Grow(int newsize);
  void Compact();

private:

  char *m_buf;
  int m_alloc;
};


#endif//_GAYSTRING_H_