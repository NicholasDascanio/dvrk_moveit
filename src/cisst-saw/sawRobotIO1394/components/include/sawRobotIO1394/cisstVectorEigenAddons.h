
  template<typename OtherT>
inline void Assign(const OtherT &other)
{
  *this = other;
}

  template<typename OtherDerived>
  void
  ElementwiseMultiply(const PlainObjectBase<OtherDerived> &other)
{
  *this = this->cwiseProduct(other);
}

  template<typename OtherDerived1, typename OtherDerived2>
  void
  ElementwiseProductOf(const PlainObjectBase<OtherDerived1> &other1, const PlainObjectBase<OtherDerived2> &other2)
{
  *this = other1.cwiseProduct(other2);
}

  template<typename OtherDerived1, typename OtherDerived2>
  void
  SumOf(const PlainObjectBase<OtherDerived1> &other1, const PlainObjectBase<OtherDerived2> &other2)
{
  *this = other1 + other2;
}


inline std::string ToString() {
  return std::string("");
}
