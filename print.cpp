/* 
 * @BEGIN LICENSE
 *
 * Copyright (C) 2014-2015  Ward Poelmans
 *
 * This file is part of v2DM-DOCI.
 * 
 * v2DM-DOCI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * v2DM-DOCI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with v2DM-DOCI.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @END LICENSE
 */

#include <iostream>
#include <fstream>
#include <getopt.h>
#include <signal.h>

#include "include.h"
#include "BoundaryPoint.h"

// from CheMPS2
#include "Hamiltonian.h"
#include "OptIndex.h"
#include "OrbitalTransform.h"
#include "UnitaryMatrix.h"

// if set, the signal has been given to stop the calculation and write current step to file
sig_atomic_t stopping = 0;
sig_atomic_t stopping_min = 0;

int main(int argc,char **argv)
{
   using std::cout;
   using std::endl;
   using namespace doci2DM;

   cout.precision(10);

   std::string inputfile = "rdm.h5";
   std::string integralsfile;
   std::string unitary;
   bool dm2 = false;
   bool dm1 = false;
   bool sorted = false;

   struct option long_options[] =
   {
      {"integrals",  required_argument, 0, 'i'},
      {"unitary",  required_argument, 0, 'u'},
      {"rdm",  required_argument, 0, 'r'},
      {"2dm",  no_argument, 0, '2'},
      {"1dm",  no_argument, 0, '1'},
      {"sort",  no_argument, 0, 's'},
      {"help",  no_argument, 0, 'h'},
      {0, 0, 0, 0}
   };

   int i,j;

   while( (j = getopt_long (argc, argv, "h12si:u:r:", long_options, &i)) != -1)
      switch(j)
      {
         case 'h':
         case '?':
            cout << "Usage: " << argv[0] << " [OPTIONS]\n"
               "\n"
               "    -r, --rdm=rdm-file              Read this RDM\n"
               "    -i, --integrals=integrals-file  Set the input integrals file\n"
               "    -u, --unitary=unitary-file      Use the unitary matrix in this file\n"
               "    -1, --1dm                       Print the 1DM\n"
               "    -2, --2dm                       Print the 2DM\n"
               "    -s, --sort                      Sort the 1DM elements\n"
               "    -h, --help                      Display this help\n"
               "\n";
            return 0;
            break;
         case 'i':
            integralsfile = optarg;
            break;
         case 'u':
            unitary = optarg;
            break;
         case 'r':
            inputfile = optarg;
            break;
         case '1':
            dm1 = true;
            break;
         case '2':
            dm2 = true;
            break;
         case 's':
            sorted = true;
            break;
      }

   cout << "Reading: " << inputfile << endl;

   int N, L;

   std::unique_ptr<CheMPS2::Hamiltonian> ham;

   if(!integralsfile.empty())
   {
      cout << "Reading ham: " << integralsfile << endl;
      ham.reset(new CheMPS2::Hamiltonian(CheMPS2::Hamiltonian::CreateFromH5(integralsfile)));

      L = ham->getL(); //dim sp hilbert space
      N = ham->getNe(); //nr of particles

      cout << "Found L=" << L << " N=" << N << endl;
   }

   TPM rdm = TPM::CreateFromFile(inputfile);
   L = rdm.gL();
   N = rdm.gN();

   cout << "Read: L=" << L << " N=" << N << endl;

   if(!integralsfile.empty() && !unitary.empty())
   {
      simanneal::OrbitalTransform orbtrans(*ham);

      orbtrans.get_unitary().loadU(unitary);
      orbtrans.fillHamCI(*ham);

      doci2DM::BoundaryPoint method(*ham);
      method.getRDM() = rdm;

      method.energyperirrep(*ham, true);

      if(dm1)
      {
         SPM spm(rdm);
         simanneal::OptIndex index(*ham);

         spm.Particlesperirrep(index);
      }

      return 0;
   }

   if(dm2)
   {
      cout << "2DM:" << endl;
      cout << rdm << endl;
      cout << "Trace: " << rdm.trace() << endl;
      cout << "Block: " << rdm.getMatrix(0).trace() << endl;
      cout << "vec: " << rdm.getVector(0).trace() << endl;
   }

   SPM spm(rdm);

   if(dm1)
   {
      cout << "1DM:" << endl;

      if(sorted)
         spm.PrintSorted();
      else
         cout << spm << endl;

      cout << "Trace: " << spm.trace() << endl;
   }

   return 0;
}


/* vim: set ts=3 sw=3 expandtab :*/
