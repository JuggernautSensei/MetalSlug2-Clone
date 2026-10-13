#pragma once

class ParamValue
{
public:
    enum class eType
    {
        Null,
        Bool,
        Number,
        String,
        Array,
        Object
    };

    [[nodiscard]] static ParamValue        MakeBool(bool _bValue);
    [[nodiscard]] static ParamValue        MakeNumber(double _value);
    [[nodiscard]] static ParamValue        MakeString(String _value);
    [[nodiscard]] static ParamValue        MakeArray();
    [[nodiscard]] static ParamValue        MakeObject();
    void                                   PushBack(ParamValue _value);
    void                                   Set(String _key, ParamValue _value);
    [[nodiscard]] bool                     IsNumber() const;
    [[nodiscard]] bool                     IsString() const;
    [[nodiscard]] bool                     IsArray() const;
    [[nodiscard]] bool                     IsObject() const;
    [[nodiscard]] bool                     AsBool(bool _bDefault = false) const;
    [[nodiscard]] float                    AsFloat(float _default = 0.f) const;
    [[nodiscard]] int                      AsInt(int _default = 0) const;
    [[nodiscard]] const String&            AsString() const;
    [[nodiscard]] size_t                   GetSize() const;
    [[nodiscard]] const ParamValue&        At(size_t _index) const;
    [[nodiscard]] const ParamValue*        Find(StringView _key) const;
    [[nodiscard]] bool                     Has(StringView _key) const;
    [[nodiscard]] bool                     GetBool(StringView _key, bool _bDefault = false) const;
    [[nodiscard]] int                      GetInt(StringView _key, int _default = 0) const;
    [[nodiscard]] String                   GetString(StringView _key, StringView _default = {}) const;
    [[nodiscard]] VECTOR2                  GetVec2(StringView _key, VECTOR2 _default = { 0.f, 0.f }) const;
    [[nodiscard]] bool                     TryGetVec2(VECTOR2& _out) const;
    [[nodiscard]] static const ParamValue& EmptyObject();
    [[nodiscard]] static const ParamValue& NullValue();

private:
    eType              m_type   = eType::Null;
    bool               m_bValue = false;
    double             m_number = 0.0;
    String             m_string = {};
    Vector<ParamValue> m_items  = {};
    Vector<String>     m_keys   = {};
};
