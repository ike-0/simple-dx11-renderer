#pragma once


class IBuffer 
{
public:
	inline IBuffer() {};
	inline IBuffer(UINT access, D3D11_USAGE usage, D3D11_BIND_FLAG flags)
	: _access(access), _usage(usage), _flags(flags) { };
	virtual ~IBuffer() = default;

	Microsoft::WRL::ComPtr<ID3D11Buffer>	_buffer;
	UINT									_size;
	UINT									_access;
	D3D11_USAGE								_usage;
	D3D11_BIND_FLAG							_flags;

	inline void SetName(const char* name)
	{
		DX::SetDebugName(_buffer.Get(), name);
	}
	inline void SetName(const std::string& name)
	{
		DX::SetDebugName(_buffer.Get(), name);
	}
	inline D3D11_USAGE GetUsage() {
		return _usage;
	}
	ID3D11Buffer* Get() const {
		return _buffer.Get();
	}
	ID3D11Buffer* const* GetAddressOf() const {
		return _buffer.GetAddressOf();
	}

protected:

};

template <typename T>
struct Buffer : public IBuffer
{
public:
	// Empty
	Buffer() {}

	// Creation
	Buffer(D3D11_BIND_FLAG flags, D3D11_USAGE usage = D3D11_USAGE::D3D11_USAGE_DEFAULT, UINT cpuaccess = 0)
		: IBuffer(cpuaccess, usage, flags)
	{
		this->_size = 1;
		Create();
	}
	// Creation
	Buffer(const std::vector<T>& initialdata, D3D11_BIND_FLAG flags, D3D11_USAGE usage = D3D11_USAGE::D3D11_USAGE_DEFAULT, UINT cpuaccess = 0)
		: IBuffer(cpuaccess, usage, flags)
	{
		this->_size = static_cast<UINT>(initialdata.size());
		Create(initialdata);
	}
	
	~Buffer() {
		Reset();
	}

	inline D3D11_MAPPED_SUBRESOURCE Map(D3D11_MAP maptype, UINT subresource = 0)
	{
#ifdef _DEBUG
		assert(_usage != D3D11_USAGE::D3D11_USAGE_IMMUTABLE); // Must be mutable buffer
		assert(_buffer.Get() != 0); // Empty Resource
		assert(_access != 0);		// No Access
#endif
		D3D11_MAPPED_SUBRESOURCE mappedregion;
		CHECK(DX::States::Context->Map(_buffer.Get(), subresource, maptype, NULL, &mappedregion));
		return mappedregion;
	}
	void Unmap(UINT subresource = 0)
	{
		DX::States::Context->Unmap(_buffer.Get(), subresource);
	}

	// Do not unmap
	inline void MapFromStaging(const Buffer<T>& buffer)
	{
#ifdef _DEBUG
		assert(buffer.GetUsage() == D3D11_USAGE::D3D11_USAGE_STAGING);
		assert(this->_usage != D3D11_USAGE::D3D11_USAGE_IMMUTABLE);
#endif
		DX::States::Context->CopyResource(_buffer.Get(), buffer.GetRawBuffer());
	}

	void Reset() {
		this->_buffer.Reset();
		this->_access = 0;
	}

	UINT Stride() const
	{
		return sizeof(T);
	}
	UINT Size() const
	{
		return this->_size;
	}




private:
	void Create(const std::vector<T>& data)
	{
		// Descriptor
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = static_cast<UINT>(sizeof(T) * data.size());
		desc.CPUAccessFlags = this->_access;
		desc.BindFlags = this->_flags;
		desc.MiscFlags = 0;
		desc.Usage = this->_usage;

		// Resource data description
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = data.data();
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		DX::States::Device->CreateBuffer(&desc, &initData, this->_buffer.ReleaseAndGetAddressOf());
	}
	void Create()
	{
		// Descriptor
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = sizeof(T);
		desc.CPUAccessFlags = this->_access;
		desc.BindFlags = this->_flags;
		desc.MiscFlags = 0;
		desc.Usage = this->_usage;

		DX::States::Device->CreateBuffer(&desc, nullptr, this->_buffer.ReleaseAndGetAddressOf());
	}
private:

};