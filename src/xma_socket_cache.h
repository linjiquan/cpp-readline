#pragma once

#include <cstring>

namespace xma {

class CPngConnBuf;

using SocketBuff = CPngConnBuf;

//socket cache for stream reading/writing
//just copied from png client
//this cache should be optimized to reduce the memmove calls
// TBD, brant, 2019.01.29
// 1 memmove
// 2 max size

//连接的读写缓冲区
class CPngConnBuf {
#define CONNECTION_LEN 4096              // 4K

public:
  CPngConnBuf(double rate, int maxBufLen/*int maxBufLen = Png_RWBUFLEN*2*/)
      : m_buf(nullptr),
        m_len(0),
        m_canRate(rate),
        m_canLen(0),
        m_index(0),
        m_maxBufLen(maxBufLen) {
    recomputeCanLen();

    // setNewSize(1024*50);//默认为50k
    setNewSize(512);  //默认为512
  }

  ~CPngConnBuf() {
    delete[] m_buf;
  }


  //拷贝buf到conn中
  inline bool copyBuf(const char* buf, int len) {
    if (len > getFreeLen()) {
      expand(len);
      if (len > getFreeLen()) {
        return false;
      }
    }

    memcpy(m_buf + m_index, buf, len);
    m_index += len;

    return true;
  }

  //对缓冲区扩容4K，
  void expand(int expandLen = CONNECTION_LEN) {
    if (expandLen <= 0) {
      return;
    }

    int newLen = m_len;
    while (1) {
      newLen *= 2;
      if (newLen > m_maxBufLen) {
        newLen = m_maxBufLen;
        break;
      }

      if (newLen >= m_len + expandLen) {
        break;
      }
    }

    if (newLen > m_len) {
      setNewSize(newLen);
    }
  }

  //申请缓冲区，调整长度,可以增大，可以缩小
  inline bool setNewSize(int len) {
    if (m_len != 0) {
      // printf("setNewSize old=>new %d=>%d\n",m_len,len);
    }

    if (len > m_len)  //增加长度
        {
      return reallocBuf(len);
    } else if (len == m_len)  //长度保持不变，什么都不用做
        {
      return true;
    } else  //缩容
    {
      if (len <= m_index)  //无法缩容
          {
        return false;
      } else {
        return reallocBuf(len);
      }
    }
  }

  //获取到缓冲区
  inline char* getBuf() const {
    return m_buf;
  }

  //获取到缓冲区的长度
  inline int getLen() const {
    return m_len;
  }

  //获取针对总容量来说还可以读写的字节数
  //==0表示已经用完
  //注意这个函数可能会做扩容操作，这样前面如果有调用getBuf这样的函数是失效了
  inline int getFreeLen() {
    int left = m_len - m_index;
    if (left < 0) {
      left = 0;
    }

    /*
     if( recurse )
     {
     if( left < 1024 )//<1k了，可以扩容
     {
     expand(4096);//扩容吧
     return getFreeLen(false);
     }
     }
     */

    return left;
  }

  //获取到当前索引
  inline int getIndex() const {
    return m_index;
  }

  //调整索引，不做安全检测，业务自己保证不要超过范围
  inline void setIndex(int index) {
    m_index = index;
  }

  //增加len个字节到索引，注意len可能为负数，表示减少
  //不做安全检测，业务自己保证不要超过范围
  //读取缓冲区使用
  inline void addIndex(int len) {
    m_index += len;

    if (getFreeLen() < CONNECTION_LEN) {
      expand(CONNECTION_LEN);
    }
  }

  //从前面删除len个字符，比如已经发送了len个字符
  //不做安全检测，业务自己保证不要超过范围
  inline void shrinkFromFront(int len) {
    int left = m_index - len;
    if (left > 0) {
      memmove(m_buf, m_buf + len, left);
    }
    m_index = left;
  }

  //获取阀值的长度，比真正的buf缓冲区长度要小，一般用在写模式下
  inline int getCanLen() const {
    return m_canLen;
  }

  //检测增加len个字节是否不超过阀值
  inline bool checkCanOp(int len) {
    if (m_index + len > m_canLen) {
      expand(m_index + len - m_canLen + CONNECTION_LEN);

      return m_index + len <= m_canLen;
    } else {
      return true;
    }
  }

 private:
  char* m_buf;       //实际的缓冲区
  int m_len;         //实际的长度
  double m_canRate;  //可写的比率 m_canLen=m_len*m_canRate;
  int m_canLen;      //可以使用的阀值长度，一般达到这个长度就开始扩容或者直接失败
  int m_index;       //读写位置

  int m_maxBufLen;  //缓冲区的最大长度

  inline void recomputeCanLen()  //计算can长度
  {
    m_canLen = (int) (m_len * m_canRate);
    if (m_canLen > 16) {
      m_canLen -= 16;  //这样比总的容量总是要少一点
    }
  }

  bool reallocBuf(int len) {
    char* newbuf = new char[len + 8];
    if (newbuf) {
      memcpy(newbuf, m_buf, m_index);  //拷贝老缓冲区的内容
      m_len = len;

      delete[] m_buf;
      m_buf = newbuf;

      recomputeCanLen();

      return true;
    } else {
      return false;
    }
  }
};
}
