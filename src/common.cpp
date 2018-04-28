#include "common.h"

data_t CreateData()
{
    return data_t(new vector<uint8_t>());
}
data_t CreateData(string string_data)
{
    return CreateData(string_data.data(), string_data.length());
}
data_t CreateData(const void* ptr, int len)
{
    data_t d = CreateData();
    d->resize(len);
    memcpy(d->data(), ptr, len);
    return d;
}
data_t CloneData(data_t d)
{
    data_t dd = CreateData();
    dd->resize(d->size());
    memcpy(dd->data(), d->data(), d->size());
    return dd;
}
data_t ConcatData(data_t a, data_t b)
{
    data_t d = CreateData();
    d->resize(a->size() + b->size());
    memcpy(d->data(), a->data(), a->size());
    memcpy(d->data()+a->size(), b->data(), b->size());
    return d;
}
