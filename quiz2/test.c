#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;


class String
{
public:
    String()
    : _pstr(new char[5]())
    {
        _pstr += 4;
        initRefcount();
    }
 
    String(const char * pstr)
    : _pstr(new char[strlen(pstr) + 5]())
    {
        _pstr += 4;
        initRefcount();
        strcpy(_pstr, pstr);
    }
 
    String(const String & rhs)
    : _pstr(rhs._pstr)
    {
        increaseRefcount();
    }
 
    String & operator=(const String & rhs)
    {
        if(this != & rhs) // 自复制
        {
            release(); //回收左操作数的空间
            _pstr = rhs._pstr; // 进行浅拷贝
            increaseRefcount();
        }
        return *this;
    }
 
    ~String() {
        release();
    }
     
    size_t refcount() const {   return *((int *)(_pstr - 4));}
 
    size_t size() const {   return strlen(_pstr);   }
 
    const char * c_str() const {    return _pstr;   }
 
 
    //问题： 下标访问运算符不能区分读操作和写操作
    char & operator[](size_t idx)
    {
        if(idx < size())
        {
            if(refcount() > 1)
            {// 进行深拷贝
                decreaseRefcount();
                char * tmp = new char[size() + 5]();
                tmp += 4;
                strcpy(tmp, _pstr);
                _pstr = tmp;
                initRefcount();
            }
            return _pstr[idx];
        } else {
            static char nullchar = '\0';
            return nullchar;
        }
    }
 
    const char & operator[](size_t idx) const
    {
        cout << "const char & operator[](size_t) const " << endl;
        return _pstr[idx];
    }
 
private:
    void initRefcount()
    {   *((int*)(_pstr - 4)) = 1;   }
 
    void increaseRefcount()
    {   ++*((int *)(_pstr - 4)); }
     
    void decreaseRefcount()
    {   --*((int *)(_pstr - 4)); }
 
    void release() {
        decreaseRefcount();
        if(refcount() == 0)
        {
            delete [] (_pstr - 4);
            cout << ">> delete heap data!" << endl;
        }
    }
 
    friend std::ostream & operator<<(std::ostream & os, const String & rhs);
 
private:
    char * _pstr;
};
  
std::ostream & operator<<(std::ostream & os, const String & rhs)
{
    os << rhs._pstr;
    return os;
}
 
int main(void)
{
    String s1;
    String s2(s1);
    cout << "s1 = " << s1 << endl;
    cout << "s2 = " << s2 << endl;
    cout << "s1's refcount = " << s1.refcount() << endl;
 
    String s3 = "hello,world";
    String s4(s3);
 
    cout << "s3 = " << s3 << endl;
    cout << "s4 = " << s4 << endl;
    cout << "s3's refcount = " << s3.refcount() << endl;
    printf("s3's address = %p\n", s3.c_str());
    printf("s4's address = %p\n", s4.c_str());
    cout << endl;
 
    String s5 = "hello,shenzheng";
    cout << "s5 = " << s5 << endl;
    s5 = s4;
    cout << endl;
    cout << "s5 = " << s5 << endl;
    cout << "s3 = " << s3 << endl;
    cout << "s4 = " << s4 << endl;
    cout << "s3's refcount = " << s3.refcount() << endl;
    printf("s5's address = %p\n", s5.c_str());
    printf("s3's address = %p\n", s3.c_str());
    printf("s4's address = %p\n", s4.c_str());
    cout << endl;
 
    cout << "执行写操作之后:" << endl;
    s5[0] = 'X';
    cout << "s5 = " << s5 << endl;
    cout << "s3 = " << s3 << endl;
    cout << "s4 = " << s4 << endl;
    cout << "s5's refcount = " << s5.refcount() << endl;
    cout << "s3's refcount = " << s3.refcount() << endl;
    printf("s5's address = %p\n", s5.c_str());
    printf("s3's address = %p\n", s3.c_str());
    printf("s4's address = %p\n", s4.c_str());
    cout << endl;
 
    cout << "执行读操作: " << endl;
    cout << "s3[0] = " << s3[0] << endl;
    cout << "s5 = " << s5 << endl;
    cout << "s3 = " << s3 << endl;
    cout << "s4 = " << s4 << endl;
    cout << "s5's refcount = " << s5.refcount() << endl;
    cout << "s3's refcount = " << s3.refcount() << endl;
    printf("s5's address = %p\n", s5.c_str());
    printf("s3's address = %p\n", s3.c_str());
    printf("s4's address = %p\n", s4.c_str());
    cout << endl;
 
    const String s6("hello");
    cout << s6[0] << endl;
 
    return 0;
}
