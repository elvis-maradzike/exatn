/** ExaTN::Numerics: Tensor network expansion
REVISION: 2020/03/14

Copyright (C) 2018-2020 Dmitry I. Lyakh (Liakh)
Copyright (C) 2018-2020 Oak Ridge National Laboratory (UT-Battelle) **/

/** Rationale:
 (a) A tensor network expansion is an ordered linear expansion
     consisting of tensor networks with complex coefficients.
     The output tensors of all constituting tensor networks must
     be congruent, that is, have the same shape and leg direction.
     The tensor network expansion is essentially a linear combination
     of tensor network vectors in a given tensor space. The rank of
     the tensor network expansion is the rank of the output tensors
     of all constituting tensor networks (they are the same).
 (b) A tensor network expansion can either be a ket or a bra.
 (c) An inner product tensor network expansion can be formed by contracting
     one tensor network expansion with another tensor network expansion
     from the dual vector space (bra*ket, ket*bra).
 (d) A direct product tensor network expansion can be formed from
     two tensor network expansions from the same space (bra*bra, ket*ket).
 (e) A tensor network operator can be applied to a tensor network expansion,
     producing another tensor network expansion in the same space.
**/

#ifndef EXATN_NUMERICS_TENSOR_EXPANSION_HPP_
#define EXATN_NUMERICS_TENSOR_EXPANSION_HPP_

#include "tensor_basic.hpp"
#include "tensor_network.hpp"
#include "tensor_operator.hpp"

#include <string>
#include <vector>
#include <complex>
#include <memory>

#include "errors.hpp"

namespace exatn{

namespace numerics{

class TensorExpansion{
public:

 //Tensor network expansion component:
 struct ExpansionComponent{
  //Tensor network:
  std::shared_ptr<TensorNetwork> network_;
  //Expansion coefficient:
  std::complex<double> coefficient_;

  ExpansionComponent(std::shared_ptr<TensorNetwork> network,
                     std::complex<double> coefficient):
   network_(network), coefficient_(coefficient)
  {}

  ExpansionComponent(const ExpansionComponent & another){
   network_ = makeSharedTensorNetwork(*(another.network_));
   coefficient_ = another.coefficient_;
  }

  ExpansionComponent & operator=(const ExpansionComponent & another){
   network_ = makeSharedTensorNetwork(*(another.network_));
   coefficient_ = another.coefficient_;
   return *this;
  }

  ExpansionComponent(ExpansionComponent &&) noexcept = default;
  ExpansionComponent & operator=(ExpansionComponent &&) noexcept = default;
  ~ExpansionComponent() = default;
 };

 using Iterator = typename std::vector<ExpansionComponent>::iterator;
 using ConstIterator = typename std::vector<ExpansionComponent>::const_iterator;

 /** Constructs an empty ket tensor expansion. **/
 TensorExpansion(bool ket = true): ket_(ket) {}

 /** Constructs a tensor expansion by applying a tensor network operator
     to another tensor network expansion. **/
 TensorExpansion(const TensorExpansion & expansion,       //in: tensor network expansion in some tensor space
                 const TensorOperator & tensor_operator); //in: tensor network operator

 /** Either constructs the inner product tensor network expansion by closing
     one tensor network expansion with another tensor network expansion
     from the dual tensor space or constructs the direct product tensor network
     expansion from two tensor network expansions from the same space. **/
 TensorExpansion(const TensorExpansion & left_expansion,   //in: tensor network expansion in some tensor space
                 const TensorExpansion & right_expansion); //in: tensor network expansion from the same or dual space

 /** Constructs the inner product tensor network expansion by applying
     a tensor network operator to a tensor network expansion (right_expansion)
     and then closing the resulting tensor network expansion with another
     tensor network expansion from the dual tensor space. **/
 TensorExpansion(const TensorExpansion & left_expansion,  //in: tensor network expansion in some tensor space
                 const TensorExpansion & right_expansion, //in: tensor network expansion from the dual tensor space
                 const TensorOperator & tensor_operator); //in: tensor network operator

 /** Produces a derivative tensor expansion by differentiating
     the tensor expansion with respect to a given tensor (by its name). **/
 TensorExpansion(const TensorExpansion & expansion, //in: original tensor expansion
                 const std::string & tensor_name,   //in: the name of the tensor which the derivative is taken against
                 bool conjugated = false);          //in: whether or not to differentiate with respect to conjugated tensors with the given name

 /** Clones the given tensor network expansion. By default, the output
     tensor in each tensor network component of the newly created tensor
     network expansion will be reset to a new one. The name of the newly
     created tensor network expansion will be the same as the original one,
     unless the new name is explicitly provided. **/
 TensorExpansion(const TensorExpansion & another,    //in: another tensor expansion to clone
                 bool reset_output_tensors,          //in: whether or not to replace output tensors in all cloned tensor networks
                 const std::string & new_name = ""); //in: new name for the cloned tensor network expansion

 TensorExpansion(const TensorExpansion &) = default;
 TensorExpansion & operator=(const TensorExpansion &) = default;
 TensorExpansion(TensorExpansion &&) noexcept = default;
 TensorExpansion & operator=(TensorExpansion &&) noexcept = default;
 virtual ~TensorExpansion() = default;

 /** Clones the current tensor network expansion. By default, the output
     tensor in each tensor network component of the newly cloned tensor
     network expansion will be reset to a new one. The name of the cloned
     tensor network expansion will be the same as the original one,
     unless the new name is explicitly provided. **/
 virtual TensorExpansion clone(bool reset_output_tensors = true, //in: whether or not to replace output tensors in all cloned tensor networks
                               const std::string & new_name = "") const; //in: new name for the cloned tensor network expansion

 inline Iterator begin() {return components_.begin();}
 inline Iterator end() {return components_.end();}
 inline ConstIterator cbegin() const {return components_.cbegin();}
 inline ConstIterator cend() const {return components_.cend();}

 /** Returns whether the tensor network expansion is ket or not. **/
 inline bool isKet() const{
  return ket_;
 }

 /** Returns whether the tensor network expansion is bra or not. **/
 inline bool isBra() const{
  return !ket_;
 }

 /** Returns the tensor network expansion name (may be empty). **/
 inline const std::string & getName() const{
  return name_;
 }

 /** Renames the tensor expansion. **/
 void rename(const std::string & name);

 /** Returns the rank of the tensor expansion (number of legs per component).
     If the expansion is empty, returns -1. **/
 inline int getRank() const{
  if(!(components_.empty())) return components_[0].network_->getRank();
  return -1;
 }

 /** Returns the total number of components in the tensor network expansion. **/
 inline std::size_t getNumComponents() const{
  return components_.size();
 }

 /** Returns a specific component of the tensor network expansion. **/
 inline const ExpansionComponent & getComponent(std::size_t component_num){
  assert(component_num < components_.size());
  return components_[component_num];
 }

 /** Appends a new component to the tensor network expansion. **/
 bool appendComponent(std::shared_ptr<TensorNetwork> network,  //in: tensor network
                      const std::complex<double> coefficient); //in: expansion coefficient

 /** Appends another tensor network expansion into the current one. **/
 bool appendExpansion(const TensorExpansion & another,         //in: tensor network expansion
                      const std::complex<double> coefficient); //in: scaling coefficient (applies to the appended tensor network expansion)

 /** Conjugates the tensor network expansion: All constituting tensors are complex conjugated,
     all tensor legs reverse their direction, complex linear expansion coefficients are conjugated:
     The ket tensor network expansion becomes a bra, and vice versa. **/
 void conjugate();

 /** Prints. **/
 void printIt() const;

private:

 /** Internal methods: **/
 void constructDirectProductTensorExpansion(const TensorExpansion & left_expansion,
                                            const TensorExpansion & right_expansion);
 void constructInnerProductTensorExpansion(const TensorExpansion & left_expansion,
                                           const TensorExpansion & right_expansion);
 bool reorderProductLegs(TensorNetwork & network,
                         const std::vector<std::pair<unsigned int, unsigned int>> & new_legs);

protected:

 /** Data members: **/
 bool ket_; //ket or bra
 std::vector<ExpansionComponent> components_; //ordered components of the tensor network expansion
 std::string name_; //tensor expansion name (optional)
};

} //namespace numerics

/** Creates a new tensor network expansion as a shared pointer. **/
template<typename... Args>
inline std::shared_ptr<numerics::TensorExpansion> makeSharedTensorExpansion(Args&&... args)
{
 return std::make_shared<numerics::TensorExpansion>(std::forward<Args>(args)...);
}

} //namespace exatn

#endif //EXATN_NUMERICS_TENSOR_EXPANSION_HPP_
