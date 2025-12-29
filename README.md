# pkrbot

Fast poker hand evaluator. Drop-in replacement for eval7, 2-3x faster.

## Install

```bash
pip install pkrbot
```

## Usage

```python
import pkrbot
import random

# Evaluate a hand
hand = [pkrbot.Card('As'), pkrbot.Card('Kh'), pkrbot.Card('Qd'), pkrbot.Card('Jc'), pkrbot.Card('Ts')]
result = pkrbot.evaluate(hand)
print(pkrbot.handtype(result))  # "Straight"

# Use a deck
deck = pkrbot.Deck()
deck.shuffle()
hand = deck.deal(7)
result = pkrbot.evaluate(hand)

# Use a set seed deck
rng = random.Random(42)
deck = pkrbot.Deck(42)
deck.shuffle()            # will use seed 42
hand = deck.sample(7)     # will use seed 42 as well
deck_2 = pkrbot.Deck(rng) # will use seed 42, but with persisted random state in rng
print(pkrbot.handtype(pkrbot.evaluate(hand)))
```

## API

**Cards**: `pkrbot.Card('As')` - Ranks: `2-9, T, J, Q, K, A`, Suits: `c, d, h, s`

**Deck**: 
- `deck.shuffle()` - Randomize
- `deck.deal(n)` - Remove and return n cards
- `deck.peek(n)` - View top n cards
- `deck.sample(n)` - Get n random cards

**Evaluate**: `result = pkrbot.evaluate(hand)` - Higher values = better hands

**Hand Type**: `pkrbot.handtype(result)` - Returns: "Straight Flush", "Quads", "Full House", "Flush", "Straight", "Trips", "Two Pair", "Pair", "High Card"

## Performance

~2-3x faster than eval7. Optimized Cython with aggressive compiler flags.

## License

MIT
