import tensorflow as tf
from keras import saving
from tensorflow import keras


@saving.register_keras_serializable()
class GameStateWeightedLoss(tf.keras.losses.Loss):
    """
    Custom loss function that directly guides the gradient based on game outcome.

    The `y_true` tensor is expected to have a shape of (batch_size, 3),
    with columns for [true_score, move_number, total_moves].
    """

    def __init__(self, name="game_state_weighted_loss", **kwargs):
        super().__init__(name=name, **kwargs)

    def call(self, y_true, y_pred):
        y_true = tf.cast(y_true, dtype=tf.float32)
        y_pred = tf.cast(y_pred, dtype=tf.float32)

        y_score = y_true[:, 0]
        y_win_loss = tf.sign(y_score)
        move_number = y_true[:, 1]
        total_moves = y_true[:, 2]
        difference = 1 - y_win_loss * y_pred
        coefficient = tf.math.abs(y_score) * tf.math.sqrt(tf.math.divide_no_nan(move_number, total_moves))
        loss = difference * coefficient
        return tf.reduce_mean(loss)
