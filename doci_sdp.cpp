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
#include <getopt.h>
#include <cassert>
#include <signal.h>

#include "include.h"
#include "PotentialReducation.h"
#include "LocalMinimizer.h"

// from CheMPS2
#include "Hamiltonian.h"

// if set, the signal has been given to stop the calculation and write current step to file
sig_atomic_t stopping = 0;
sig_atomic_t stopping_min = 0;

void stopcalcsignal(int sig);
void stopminsignal(int sig);


int main(int argc, char **argv)
{
   using std::cout;
   using std::endl;
   using namespace doci2DM;
   using simanneal::LocalMinimizer;

   cout.precision(10);

   std::string integralsfile = "mo-integrals.h5";
   std::string unitary;
   bool random = false;
   bool localmini = false;
   bool scan = false;

   struct option long_options[] =
   {
      {"integrals",  required_argument, 0, 'i'},
      {"unitary",  required_argument, 0, 'u'},
      {"random",  no_argument, 0, 'r'},
      {"scan",  no_argument, 0, 's'},
      {"local-minimizer",  no_argument, 0, 'l'},
      {"help",  no_argument, 0, 'h'},
      {0, 0, 0, 0}
   };

   int i,j;

   while( (j = getopt_long (argc, argv, "d:rlhi:u:s", long_options, &i)) != -1)
      switch(j)
      {
         case 'h':
         case '?':
            cout << "Usage: " << argv[0] << " [OPTIONS]\n"
               "\n"
               "    -i, --integrals=integrals-file  Set the input integrals file\n"
               "    -u, --unitary=unitary-file      Use the unitary matrix in this file\n"
               "    -r, --random                    Perform a random unitary transformation on the Hamiltonian\n"
               "    -l, --local-minimizer           Use the local minimizer\n"
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
            random = true;
            break;
         case 'l':
            localmini = true;
            break;
         case 's':
            scan = true;
            break;
      }

   cout << "Reading: " << integralsfile << endl;

   // make sure we have a save path, even if it's not specify already
   // This will not overwrite an already set SAVE_H5_PATH
   setenv("SAVE_H5_PATH", "./", 0);

   cout << "Using save path: " << getenv("SAVE_H5_PATH") << endl;

   auto ham = CheMPS2::Hamiltonian::CreateFromH5(integralsfile);

   const auto L = ham.getL(); //nr of particles
   const auto N = ham.getNe(); //nr of particles

   cout << "Starting with L=" << L << " N=" << N << endl;

   if(!unitary.empty() && !localmini)
   {
      cout << "Reading transform: " << unitary << endl;

      simanneal::OrbitalTransform orbtrans(ham);

      orbtrans.get_unitary().loadU(unitary);
      orbtrans.fillHamCI(ham);
   }

   const simanneal::OptIndex opt(ham);

   if(random)
   {
      simanneal::UnitaryMatrix X(opt);
      X.fill_random();
      X.make_skew_symmetric();
      X.print_unitary();
      simanneal::OrbitalTransform orbtrans(ham);
      orbtrans.update_unitary(X, false);
      std::string filename = getenv("SAVE_H5_PATH");
      filename += "/random-start-unitary.h5";
      orbtrans.get_unitary().saveU(filename);
      orbtrans.fillHamCI(ham);
   }

   PotentialReduction method(ham);

   // set up everything to handle SIGALRM
   struct sigaction act;
   act.sa_flags = 0;
   act.sa_handler = &stopcalcsignal;

   sigset_t blockset;
   sigemptyset(&blockset); // we don't block anything in the handler
   act.sa_mask = blockset;

   sigaction(SIGALRM, &act, 0);

   act.sa_handler = &stopminsignal;

   sigaction(SIGUSR1, &act, 0);

   if(localmini)
   {
      LocalMinimizer minimize(ham);

      if(!unitary.empty())
      {
         cout << "Starting local minimizer from: " << unitary << endl;
         minimize.getOrbitalTf().get_unitary().loadU(unitary);
      }

      minimize.UsePotentialReduction();

      minimize.set_conv_crit(1e-6);

      minimize.Minimize();

      cout << "Bottom is " << minimize.get_energy() << endl;

      method = minimize.getMethod_PR();
      ham = minimize.getHam();
   } else
      method.Run();

   cout << "The optimal energy is " << method.evalEnergy() << std::endl;

   if(scan)
      Tools::scan_all(method.getRDM(), ham);

   std::string h5_name = getenv("SAVE_H5_PATH");
   h5_name += "/optimal-rdm.h5";

   method.getRDM().WriteToFile(h5_name);

   h5_name = getenv("SAVE_H5_PATH");
   h5_name += "/optimal-ham.h5";

   ham.save2(h5_name);

   return 0;
}

void stopcalcsignal(int sig)
{
   stopping=1;
}

void stopminsignal(int sig)
{
   stopping_min=1;
}

/*  vim: set ts=3 sw=3 expandtab :*/
