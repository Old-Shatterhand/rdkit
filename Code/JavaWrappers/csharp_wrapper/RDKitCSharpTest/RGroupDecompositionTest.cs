﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using GraphMolWrap;
using Xunit;

namespace RdkitTests
{
    public class RGroupDecompositionTest
    {
        [Fact]
        public void TestAllowTautomer()
        {
            var block = @"
  Mrv2008 08072313382D          

  9  9  0  0  0  0            999 V2000
    5.9823    5.0875    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.9823    4.2625    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.2679    3.8500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    4.5534    4.2625    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    4.5534    5.0875    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0
    5.2679    5.5000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.2679    6.3250    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
    6.6968    3.8500    0.0000 R#  0  0  0  0  0  0  0  0  0  0  0  0
    5.2679    3.0250    0.0000 R#  0  0  0  0  0  0  0  0  0  0  0  0
  1  2  2  0  0  0  0
  2  3  1  0  0  0  0
  3  4  2  0  0  0  0
  4  5  1  0  0  0  0
  5  6  1  0  0  0  0
  6  7  2  0  0  0  0
  1  6  1  0  0  0  0
  2  8  1  0  0  0  0
  3  9  1  0  0  0  0
M  RGP  2   8   1   9   2
M  END";
            var core = RWMol.MolFromMolBlock(block);

            var rgdParameters = new RGroupDecompositionParameters {
                matchingStrategy = (uint) RGroupMatching.GreedyChunks,
                scoreMethod = (uint) RGroupScore.FingerprintVariance,
                onlyMatchAtRGroups = false,
                removeHydrogensPostMatch = true,
                removeAllHydrogenRGroups = true,
                allowMultipleRGroupsOnUnlabelled = true,
                doTautomers = true
            };

            var cores = new ROMol_Vect();
            cores.Add(core);
            var rgd = new RGroupDecomposition(cores, rgdParameters);
            var mol1 = RWMol.MolFromSmiles("Cc1cnc(O)cc1Cl");
            var mol2 = RWMol.MolFromSmiles("CC1=CNC(=O)C=C1F");
            Assert.Equal(0, rgd.add(mol1));
            Assert.Equal(1, rgd.add(mol2));
            Assert.True(rgd.process());

            var rows = rgd.getRGroupsAsRows();
            Assert.Equal(2, rows.Count);
        }
    }
}
