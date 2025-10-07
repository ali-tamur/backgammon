
# AI Backgammon Project Roadmap

## (P0) C++ Core Mechanics & Performance Improvements
- [+] Move Generation: Complete C++ unit tests
- [ ] (P3) C++ Game Manager: Carry the core C++ game loop to C++ 
- [+] Performance Improvements: Optimize the loop to play 1000 games/second (with placeholder player)
- [+] Integration tests fo Move Generation: Complete C++ unit tests
- [+] Implement the Pybind11
- [+] Update the log schema: Every move should be an independent record.

## (P1) Python Bridge & Simple NN Models
- [+] Use Pybind11 to call the C++ move generation from Python game manager.
- [ ] Integration tests for the pybind11 bridge.
- [ ] Use tensorflow/keras to create a simple feedforward NN
- [ ] Experiment with meta parameters; can we have a model that is
      significantly better than a random player.
- [ ] Design a structure to keep experiment parameters and results

## (P2) Productionization
- [ ] Implement the docker layer so that the whole project (c++ and python) can be
      uploaded anywhere.
- [ ] Implement the kubernetes layer for a performant training pipeline.

## (P4) Web Interface
- [ ] Make a very simple interface to let a model play with a human player.
