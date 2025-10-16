
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

## NN Design Decisions:
1. NN input will always be from White point of view and when it is black's turn to move. With this representation we will not need to encode "whose turn is it", whihc is good because
that single input feature would have a disproportionate affect on the output.

2. We will only train for the white player so + scores will always mean win, and - scores
will always mean loss. Corollary: the score of a position P with white to move is equal to the reverse score of the position with the pieces flipped and it is black to move.

3. As a consequence, we need to update game manager so that the black player will start the
game 50% of the time.

4. The first toy model has a single output feature, but having a 4-tuple as in TD-backgammon
is probably better: P(W win), P(B win), P(W gammon), P(B gammon) we can represent backgammons
results simply as gammon results.

5. There will be 2 types of training data with associated loss functions:
* The Game Result: The loss depends on the actual game result. If we adapt 4th decision, the output vector will simply represent probabilities, simplifying the semantics. The affect of the game score on a particular position will depend on how close the game end is. We need to experiment to choose a good middle ground so that the derivatives for the early moves will neither be too large, nor too small.
* Temporal Difference: Given a position P, let P_max(d1, d2) represent the best position the player can achieve on a dice throw of d1, d2. Then the evaluation of P should equal to the expected score of all those positions on all possible dice throws.

# Diary

## Oct 7, 2025: Basic NN beats random player 1276-468 in 1000 games (73% win rate)

**Model Architecture:**
- Input: 250 features (one-hot encoding of board state)
- Hidden: 128 → 64 → 64 → 32 (relu)
- Output: 1 neuron (tanh, outputs position evaluation score)

**Training:**
- Data: 110K games of random vs random play (~10.5M positions)
- Loss: Custom weighted loss (win/loss signal × game_score × move_timing)
- Optimizer: Adam (learning rate=0.001)
- Regularization: L2 - 0.00005
- Batch size: 32 (critical - larger batches failed to learn)
- Epochs: Validation loss plateaus after 11th epoch.

**Key learnings:**
- Small batch size (32) was crucial for escaping poor local minima
- Model outputs position scores; selects move with highest score
- Loss function penalizes wrong predictions weighted by move importance

**What didn't work:**
- Large batch sizes (128, 256)
- Heavy L2 regularization (0.02)
- Smaller model capacity (128→64→32)










