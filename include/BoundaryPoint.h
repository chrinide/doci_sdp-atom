#ifndef BOUNDARY_POINT_H
#define BOUNDARY_POINT_H

#include "include.h"

namespace CheMPS2 { class Hamiltonian; }

namespace doci2DM
{

class BoundaryPoint
{
   public:

      BoundaryPoint(const CheMPS2::Hamiltonian &);

      virtual ~BoundaryPoint() = default;

      void BuildHam(const CheMPS2::Hamiltonian &);

      void Run();

      double getEnergy() const;

      void set_tol_PD(double);

      void set_tol_en(double);

      void set_mazzy(double);

      void set_sigma(double);

      void set_max_iter(int);

      SUP& getX() const;

      SUP& getZ() const;

      Lineq& getLineq() const;

   private:

      std::unique_ptr<TPM> ham;

      std::unique_ptr<SUP> X;

      std::unique_ptr<SUP> Z;

      std::unique_ptr<Lineq> lineq;

      int L;

      int N;

      double nuclrep;

      double tol_PD, tol_en;

      double mazzy;

      double sigma;

      int max_iter;

      double energy;
};

}

#endif /* BOUNDARY_POINT_H */

/* vim: set ts=3 sw=3 expandtab :*/