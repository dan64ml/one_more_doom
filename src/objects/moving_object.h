#ifndef MOVING_OBJECT_H
#define MOVING_OBJECT_H

#include "map_object.h"

namespace mobj {

class MovingObject : public MapObject {
 public:
  MovingObject(id::mobjtype_t type);

  bool TickTime() override;

  // Using with moving planes. When sector changes its height we should change
  // the mobj's position too (mostly z coordinate). If the mobj does't fit to
  // new sector height, the function returns false. Otherwise it returns true
  // and changes z coordinate according to new sizes if it's necessary.
  bool AdjustMobjHeight() override;

 protected:
  // Invoked if the object run into sth. XYMove() will be interrupted
  // immediately if return false.
  // Defines common action after hit, e.g. sliding for player, missile explosion etc...
  //  For others - just stop moving (???)
  // (new_x, new_y) - the position that XYMove() couldn't reach
  virtual bool RunIntoAction(double new_x, double new_y);

  // Contains logic for object speed reducing
  virtual void SlowDown();

  // Applied to each touched MapObject until return false
  // NB! Check for MF_SPECIAL flag
  virtual bool InfluenceObject(MapObject*) { return true; }
  
  // Applied to each crossed line until return false.
  // False means it's impossible to cross the line.
  virtual bool ProcessLine(world::Line* line);

  // Applied to each crossed special line
  virtual void ProcessSpecialLine([[maybe_unused]] world::Line* line) {}

  // It looks like a candidate to be virtual...
  virtual void ZMove();

  virtual void CallStateFunction([[maybe_unused]] id::FuncId foo_id) {}

  // Check the position, interact with items and move
  // the object to the new position if it's possible (and return true)
  // false if moving impossible
  bool TryMoveTo(double new_x, double new_y);
  
  // Iterates over all mobjs and lines in current and adjacent BlockMaps
  // Iterating lines updates opeining (tmp_* values)
  bool CheckPosition(double new_x, double new_y);

  // Crossed last step special lines
  std::vector<world::Line*> spec_lines_;
  // Pointers to obstacles that were the reason of fail of TryMoveTo()
  // TODO: obsolete???
  world::Line* line_obstacle_;
  MapObject* mobj_obstacle_;

  // Updates tmp_* variables
  void UpdateOpening(const world::Line* line);

  // what's the difference with floor_z and ceiling_z ?????
  // Just tmp values for calculating floor_z and ceiling_z.
  // TODO: bad arch, ref!!!
  double tmp_ceiling;
  double tmp_floor;
  double tmp_dropoff;
  
   // Flags that it possible to move within (!!!!TODO: tmp_ceiling - tmp_floor or ceiling_z - floor_z)
  bool float_ok_;

  // Momentums, in fact speed in strange units
  double mom_x = 0;
  double mom_y = 0;
  double mom_z = 0;

private:
  void MoveObject();
  void XYMove();

  // Checks current position and updates floor_z, ceiling_z and dropoff_z;
  void UpdateOpening();

};

} // namespace mobj

#endif // MOVING_OBJECT_H