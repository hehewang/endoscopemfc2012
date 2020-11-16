#ifndef COMIO_H
#define COMIO_H


class ComIO
{
public:
    ComIO();
    virtual ~ComIO();
private:
    virtual Initialize();
    virtual Release();
    bool SendData(unsigned char* buf,int size);
    virtual RecvDataHandle();

};

#endif // COMIO_H
