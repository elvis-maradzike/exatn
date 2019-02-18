#include "MPIClient.hpp"

namespace exatn {
namespace rpc {
namespace mpi {

MPIClient::MPIClient() {
  char portName[MPI_MAX_PORT_NAME];

  // First things first, the server is going
  // to send us the port name
  MPI_Status status;
  MPI_Recv(portName, MPI_MAX_PORT_NAME, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
  std::cout << "[mpi-client] Attempting to connect with server - " << portName << "\n";

  // Connect to the server, creating a new intercomm, serverComm
  MPI_Comm_connect(portName, MPI_INFO_NULL, 0, MPI_COMM_SELF,
                   &serverComm);
  std::cout << "[mpi-client] Connected with the server\n";
}

// Send TaProl string, get a jobId string,
// so this is an asynchronous call
const std::string MPIClient::sendTAProL(const std::string taProlStr) {

  auto jobId = generateRandomString();

  // Analyze the taProlStr to see how many GET commands there are
  // and populate the jobId2NResults map
  int nResults = 1;
  jobId2NResults.insert({jobId, nResults});

  // Asynchronously send the taProl string to the server
  MPI_Request request;
  std::cout << "[mpi-client] sending request with jobid " << jobId << "\n";
  MPI_Isend(taProlStr.c_str(), taProlStr.size(), MPI_CHAR, 0, 0, serverComm,
            &request);

  // Store the request object for us to use
  // later to wait on results in retrieveResult
  requests.insert({jobId, request});

  return jobId;
}

// Retrieve result of job with given jobId.
// Returns a scalar type double?
const std::vector<std::complex<double>> MPIClient::retrieveResult(const std::string jobId) {

  auto request = requests[jobId];

  MPI_Status status, status2;
  MPI_Wait(&request, &status);

  // now we know the execution has occurred,
  // send a synchronize command to driver
  // Tag of 1 == SYNCHRONIZE command
  char buf[1];
  MPI_Send(buf, 1, MPI_CHAR, 0, 1, serverComm);

  // now get all the results

  std::vector<std::complex<double>> results;
  for (int k = 0; k < jobId2NResults[jobId]; k++) {
    double r, i;
    MPI_Recv(&r, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, serverComm, &status2);
    MPI_Recv(&i, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, serverComm, &status2);
    results.push_back(std::complex<double>(r,i));
  }
  return results;
}

void MPIClient::shutdown() {
  char buf[1];
  MPI_Request request;
  std::cout << "[mpi-client] sending shutdown.\n";
  // Tag of 2 == SHUTDOWN command
  MPI_Isend(buf, 1, MPI_CHAR, 0, 2, serverComm, &request);
   MPI_Status status;
   std::cout << "[mpi-client] waiting for shutdown.\n";
  MPI_Wait(&request, &status);
  MPI_Comm_disconnect(&serverComm);
}

} // namespace mpi
} // namespace rpc
} // namespace exatn