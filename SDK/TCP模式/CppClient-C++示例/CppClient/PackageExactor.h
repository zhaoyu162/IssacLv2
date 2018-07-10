#pragma once
template<typename TDgsType>
class CPackageExactor
{
public:
	CPackageExactor(const char* pRawBuff)
	{
		ATLENSURE(pRawBuff != NULL);
		ptrPackage = (TDgsType*)pRawBuff;
	}

	const TDgsType* operator ->() {
		return ptrPackage;
	}

private:
	TDgsType* ptrPackage;
};

