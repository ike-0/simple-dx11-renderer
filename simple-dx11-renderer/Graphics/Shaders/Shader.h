#pragma once

class IShader
{
public:
	inline IShader() : name("null") {};
	inline IShader(const std::string& _name, const Microsoft::WRL::ComPtr<ID3DBlob>& blob)
		: _iscompiled(0), _blob(blob), name(_name) { };
	virtual ~IShader() = default;

	virtual void Compile() = 0;
	std::string name;

	inline LPVOID GetBufferPointer()
	{
		return _blob->GetBufferPointer();
	}
	inline SIZE_T GetBufferSize()
	{
		return _blob->GetBufferSize();
	}

protected:
	BOOL								_iscompiled;
	Microsoft::WRL::ComPtr<ID3DBlob>	_blob;
};


class VertexShader : public IShader {
public:
	inline VertexShader() {};
	inline VertexShader(const std::string& name, const Microsoft::WRL::ComPtr<ID3DBlob>& blob) : IShader(name, blob) {

	};
	inline ~VertexShader() {};

	inline void Compile() {
		CHECK(DX::States::Device->CreateVertexShader(this->_blob->GetBufferPointer(), this->_blob->GetBufferSize(),
			nullptr, _vs.ReleaseAndGetAddressOf()));
		this->_iscompiled = 1;
	}

	inline ID3D11VertexShader* Get() {
		return _vs.Get();
	}
	inline void Reset() {
		_vs.Reset();
	}

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	_vs;
};

class HullShader : public IShader {
public:
	inline HullShader() {};
	inline HullShader(const std::string& name, const Microsoft::WRL::ComPtr<ID3DBlob>& blob) : IShader(name, blob) {

	};
	inline ~HullShader() {};

	inline void Compile() {
		CHECK(DX::States::Device->CreateHullShader(this->_blob->GetBufferPointer(), this->_blob->GetBufferSize(),
			nullptr, _hs.ReleaseAndGetAddressOf()));
		this->_iscompiled = 1;
	}

	inline ID3D11HullShader* Get() {
		return _hs.Get();
	}
	inline void Reset() {
		_hs.Reset();
	}
private:
	Microsoft::WRL::ComPtr<ID3D11HullShader>	_hs;
};

class DomainShader : public IShader {
public:
	inline DomainShader() {};
	inline DomainShader(const std::string& name, const Microsoft::WRL::ComPtr<ID3DBlob>& blob) : IShader(name, blob) {

	};
	inline ~DomainShader() {};

	inline void Compile() {
		CHECK(DX::States::Device->CreateDomainShader(this->_blob->GetBufferPointer(), this->_blob->GetBufferSize(),
			nullptr, _ds.ReleaseAndGetAddressOf()));
		this->_iscompiled = 1;
	}

	inline ID3D11DomainShader* Get() {
		return _ds.Get();
	}
	inline void Reset() {
		_ds.Reset();
	}
private:
	Microsoft::WRL::ComPtr<ID3D11DomainShader>	_ds;
};

class GeometryShader : public IShader {
public:
	inline GeometryShader() {};
	inline GeometryShader(const std::string& name, const Microsoft::WRL::ComPtr<ID3DBlob>& blob) : IShader(name, blob) {

	};
	inline ~GeometryShader() {};

	inline void Compile() {
		CHECK(DX::States::Device->CreateGeometryShader(this->_blob->GetBufferPointer(), this->_blob->GetBufferSize(),
			nullptr, _gs.ReleaseAndGetAddressOf()));
		this->_iscompiled = 1;
	}

	inline ID3D11GeometryShader* Get() {
		return _gs.Get();
	}
	inline void Reset() {
		_gs.Reset();
	}
private:
	Microsoft::WRL::ComPtr<ID3D11GeometryShader>	_gs;
};

class PixelShader : public IShader {
public:
	inline PixelShader() {};
	inline PixelShader(const std::string& name, const Microsoft::WRL::ComPtr<ID3DBlob>& blob) : IShader(name, blob) {

	};
	inline ~PixelShader() {};

	inline void Compile() {
		CHECK(DX::States::Device->CreatePixelShader(this->_blob->GetBufferPointer(), this->_blob->GetBufferSize(),
			nullptr, _ps.ReleaseAndGetAddressOf()));
		this->_iscompiled = 1;
	}

	inline ID3D11PixelShader* Get() {
		return _ps.Get();
	}
	inline void Reset() {
		_ps.Reset();
	}
private:
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	_ps;
};

class ComputeShader : public IShader {
public:
	inline ComputeShader() {};
	inline ComputeShader(const std::string& name, const Microsoft::WRL::ComPtr<ID3DBlob>& blob) : IShader(name, blob) {

	};
	inline ~ComputeShader() {};

	inline void Compile() {
		CHECK(DX::States::Device->CreateComputeShader(this->_blob->GetBufferPointer(), this->_blob->GetBufferSize(),
			nullptr, _cs.ReleaseAndGetAddressOf()));
		this->_iscompiled = 1;
	}

	inline ID3D11ComputeShader* Get() {
		return _cs.Get();
	}
	inline void Reset() {
		_cs.Reset();
	}
private:
	Microsoft::WRL::ComPtr<ID3D11ComputeShader>	_cs;
};