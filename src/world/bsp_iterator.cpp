#include "bsp_iterator.h"

#include <cassert>

#include "fast_bsp.h"

namespace world {

BspIterator::BspIterator(const wad::FastBsp* fbsp) : fbsp_(fbsp) {
  S_.push(fbsp_->nodes_.size() - 1);
  FindNextNode();
}

void BspIterator::FindNextNode() {
  while (!S_.empty()) {
    auto node_idx = S_.top();
    S_.pop();

    // Is it a leaf?
    if (node_idx & wad::FastBsp::kNfSubsector) {
      current_leaf = node_idx & (~wad::FastBsp::kNfSubsector);
      return;
    }

    int side = fbsp_->DefineVpSide(fbsp_->vp_x_, fbsp_->vp_y_, node_idx);
    const auto& node = fbsp_->nodes_[node_idx];
    S_.push(node.children[side ^ 1]);
    S_.push(node.children[side]);
  }
}

bool BspIterator::operator==(const BspIterator& rhs) const {
  if (S_.size() != rhs.S_.size()) {
    return false;
  }
  if (S_.empty()) {
    return true;
  }
  return S_.top() == rhs.S_.top();
}

bool BspIterator::operator!=(const BspIterator& rhs) const {
  return !(*this == rhs);
}

const SubSector* BspIterator::operator*() const {
  assert(!S_.empty());
  return &((*fbsp_->ss_)[current_leaf]);
}

const BspIterator& BspIterator::operator++() {
  FindNextNode();
  return *this;
}

} // namespace world