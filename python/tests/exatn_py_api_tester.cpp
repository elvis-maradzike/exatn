#include <gtest/gtest.h>
#include <pybind11/embed.h>
#include <pybind11/eval.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>

// #include "exatn.hpp"

// using namespace exatn;
namespace py = pybind11;

TEST(ExaTN_PythonTester, checkExaTNPyAPI) {
  py::scoped_interpreter guard{};

  py::print("\n[ Test Simple ]");
  py::exec(
      R"""(
import sys
from pathlib import Path
sys.path.insert(1, str(Path.home()) + '/.exatn')
import exatn

exatn.createTensor('Z0')
exatn.createTensor('T0', [2,2], .01)
exatn.createTensor('T1', [2,2,2], .01)
exatn.createTensor('T2', [2,2], .01)
exatn.createTensor('H0', [2,2,2,2], .01)
exatn.createTensor('S0', [2,2], .01)
exatn.createTensor('S1', [2,2,2], .01)
exatn.createTensor('S2', [2,2], .01)

exatn.evaluateTensorNetwork('{0,1} 3-site MPS closure', 'Z0() = T0(a,b) * T1(b,c,d) * T2(d,e) * H0(a,c,f,g) * S0(f,h) * S1(h,g,i) * S2(i,e)')

z0 = exatn.getLocalTensor('Z0')
assert(abs(z0 - 5.12e-12) < 1e-12)

)""");

  py::print("\n[ Test Quantum Circuit Network ]");
  py::exec(
      R"""(
import sys
from pathlib import Path
sys.path.insert(1, str(Path.home()) + '/.exatn')
import exatn, numpy as np

#Quantum Circuit:
#Q0----H---------
#Q1----H----C----
#Q2----H----N----

#Define the initial qubit state vector:
qzero = np.array([1.0, 0.0], dtype=complex)
hadamard = np.array([[1., 1.],[1., -1.]], dtype=complex)

cnot = np.reshape(np.array([[1, 0, 0, 0],[0, 1, 0, 0],[0, 0, 0, 1],[0, 0, 1, 0]], dtype=complex), (2,2,2,2))

exatn.createTensor('Q0', qzero)
exatn.createTensor('Q1', qzero)
exatn.createTensor('Q2', qzero)

exatn.createTensor('H', hadamard)
exatn.createTensor('CNOT', cnot)

exatn.registerTensorIsometry('H', [0], [1])
exatn.registerTensorIsometry('CNOT', [0,1], [2,3])

circuit = exatn.TensorNetwork('QuantumCircuit')
circuit.appendTensor(1, 'Q0')
circuit.appendTensor(2, 'Q1')
circuit.appendTensor(3, 'Q2')

circuit.appendTensorGate(4, 'H', [0])
circuit.appendTensorGate(5, 'H', [1])
circuit.appendTensorGate(6, 'H', [2])

circuit.appendTensorGate(7, 'CNOT', [1,2])

circuit.printIt()

inverse = exatn.TensorNetwork(circuit)
inverse.rename('InverseCircuit')
inverse.appendTensorGate(8, 'CNOT', [1,2], True)

inverse.appendTensorGate(9, 'H', [2], True)
inverse.appendTensorGate(10, 'H', [1], True)
inverse.appendTensorGate(11, 'H', [0], True)

assert(inverse.collapseIsometries())
inverse.printIt()

assert(exatn.evaluate(circuit))

[exatn.destroyTensor('Q'+str(i)) for i in range(3)]
exatn.destroyTensor('H')
)""");

  py::print("[ Done ]");

  py::print("\n[ Test Circuit Conjugate ]");
  py::exec(
      R"""(
import sys
from pathlib import Path
sys.path.insert(1, str(Path.home()) + '/.exatn')
import exatn, numpy as np

qzero = np.array([1.0, 0.0], dtype=complex)
unitary = np.reshape(np.array([1,-1j,-1j,1], dtype=complex), (2,2))

exatn.createTensor('Q0', qzero)
exatn.createTensor('U', unitary)
exatn.registerTensorIsometry('U', [0], [1])


circuit = exatn.TensorNetwork('QuantumCircuit')
circuit.appendTensor(1, 'Q0')
circuit.appendTensorGate(2, 'U', [0])
circuit.printIt()

conj_circuit = exatn.TensorNetwork(circuit)
conj_circuit.rename('ConjugatedCircuit')
conj_circuit.conjugate()
conj_circuit.printIt()

assert(exatn.evaluate(circuit))
assert(exatn.evaluate(conj_circuit))

print(exatn.getLocalTensor(circuit.getTensor(0).getName()))
print(exatn.getLocalTensor(conj_circuit.getTensor(0).getName()))
exatn.destroyTensor('Q0')
)""");

  py::print("[ Done ]");

  py::print("\n[ Test Large Circuit ]");
  py::exec(
      R"""(
import sys
from pathlib import Path
sys.path.insert(1, str(Path.home()) + '/.exatn')
import exatn, numpy as np

qzero = np.array([1.0, 0.0], dtype=complex)
hadamard = np.array([[1., 1.],[1., -1.]], dtype=complex)

nQubits = 10
[exatn.createTensor('Q'+str(i), qzero) for i in range(nQubits)]

exatn.createTensor('H', hadamard)
exatn.registerTensorIsometry('H', [0], [1])

tensorCounter = 1
circuit = exatn.TensorNetwork('QuantumCircuit')
[circuit.appendTensor(tensorCounter+c, 'Q'+str(i)) for i,c in enumerate(range(nQubits))]
tensorCounter += nQubits

qubitReg = exatn.TensorNetwork(circuit)
qubitReg.rename('QubitKet')

[circuit.appendTensorGate(tensorCounter+c, 'H', [i]) for i, c in enumerate(range(nQubits))]
tensorCounter += nQubits

circuit.printIt()

inverse = exatn.TensorNetwork(circuit)
inverse.rename('InverseCircuit')

[inverse.appendTensorGate(tensorCounter+c, 'H', [nQubits - i - 1], True) for i,c in enumerate(range(nQubits))]
tensorCounter += nQubits

assert(inverse.collapseIsometries())

inverse.printIt()


bra = qubitReg
bra.conjugate()
bra.rename('QubitBra')
pairings = [[i,i] for i in range(nQubits)]
inverse.appendTensorNetwork(bra, pairings)

inverse.printIt()
assert(inverse.getRank() == 0)

assert(exatn.evaluate(inverse))

print(exatn.getLocalTensor(inverse.getTensor(0).getName()))
[exatn.destroyTensor('Q'+str(i)) for i in range(nQubits)]
exatn.destroyTensor('H')
)""");

  py::print("[ Done ]");
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}