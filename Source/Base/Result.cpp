#include "Base/Result.h"

Result<void>::Result(const Error _error)
    : m_error(_error)
{
}

Result<void>::Result(const eError _error)
    : m_error(_error)
{
}

bool Result<void>::IsOK() const
{
    return m_error.IsOK();
}

bool Result<void>::HasError() const
{
    return m_error.IsError();
}

Error Result<void>::GetError() const
{
    return m_error;
}

Result<void>::operator bool() const
{
    return IsOK();
}
