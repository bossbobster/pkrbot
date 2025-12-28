# cython: language_level=3
# cython: boundscheck=False
# cython: wraparound=False
# cython: cdivision=True
# cython: initializedcheck=False

from libc.stdint cimport uint8_t, uint16_t, int8_t, uint32_t
cimport cython
from libc.string cimport memset
from random import Random

# Rank and suit mappings matching eval7
RANKS = '23456789TJQKA'
SUITS = 'cdhs'  # clubs, diamonds, hearts, spades


cdef class Card:
    """
    A card with rank and suit, compatible with eval7.Card interface.
    
    Example:
        card = Card("As")  # Ace of spades
        card = Card("2c")  # 2 of clubs
    """
    cdef public uint8_t rank
    cdef public uint8_t suit
    
    def __cinit__(self, str card_string):
        if len(card_string) != 2:
            raise ValueError(f"Card string must be 2 characters, got: {card_string}")
        
        cdef str rank_char = card_string[0]
        cdef str suit_char = card_string[1]
        
        # Parse rank (0-12: 2-A)
        rank_idx = RANKS.find(rank_char)
        if rank_idx == -1:
            raise ValueError(f"Invalid rank: {rank_char}. Must be one of {RANKS}")
        self.rank = <uint8_t>rank_idx
        
        # Parse suit (0-3: c, d, h, s)
        suit_idx = SUITS.find(suit_char)
        if suit_idx == -1:
            raise ValueError(f"Invalid suit: {suit_char}. Must be one of {SUITS}")
        self.suit = <uint8_t>suit_idx
    
    def __str__(self):
        return RANKS[self.rank] + SUITS[self.suit]
    
    def __repr__(self):
        return f'Card("{self.__str__()}")'
    
    def __richcmp__(self, other, int op):
        cdef Card other_card
        cdef bint eq, gt
        
        if isinstance(other, Card):
            other_card = <Card>other
            eq = self.rank == other_card.rank and self.suit == other_card.suit
            gt = (self.rank > other_card.rank) or (self.rank == other_card.rank and self.suit > other_card.suit)
            
            if op == 0:    # <
                return not (gt or eq)
            elif op == 1:  # <=
                return not gt
            elif op == 2:  # ==
                return eq
            elif op == 3:  # !=
                return not eq
            elif op == 4:  # >
                return gt
            else:          # >=
                return gt or eq
        else:
            if op == 2:    # ==
                return False
            elif op == 3:  # !=
                return True
            else:
                raise TypeError(f"Cannot compare Card and {type(other)}")
    
    def __hash__(self):
        return (self.rank << 8) | self.suit


# Precompute straight masks
cdef uint16_t[10] straight_masks
straight_masks[0] = 0b1111100000000  # A-K-Q-J-T
straight_masks[1] = 0b0111110000000  # K-Q-J-T-9
straight_masks[2] = 0b0011111000000  # Q-J-T-9-8
straight_masks[3] = 0b0001111100000  # J-T-9-8-7
straight_masks[4] = 0b0000111110000  # T-9-8-7-6
straight_masks[5] = 0b0000011111000  # 9-8-7-6-5
straight_masks[6] = 0b0000001111100  # 8-7-6-5-4
straight_masks[7] = 0b0000000111110  # 7-6-5-4-3
straight_masks[8] = 0b0000000011111  # 6-5-4-3-2
straight_masks[9] = 0b1000000001111  # A-5-4-3-2 (wheel)

@cython.boundscheck(False)
@cython.wraparound(False)
cdef inline int max_int(int a, int b) nogil:
    return a if a > b else b

@cython.boundscheck(False)
@cython.wraparound(False)
cdef inline uint8_t max_uint8(uint8_t a, uint8_t b) nogil:
    return a if a > b else b

@cython.boundscheck(False)
@cython.wraparound(False)
cdef inline int8_t max_int8(int8_t a, int8_t b) nogil:
    return a if a > b else b

@cython.boundscheck(False)
@cython.wraparound(False)
cdef inline uint16_t max_uint16(uint16_t a, uint16_t b) nogil:
    return a if a > b else b

# evaluate function returns a uint32_t

@cython.boundscheck(False)
@cython.wraparound(False)
cpdef uint32_t evaluate_raw(uint8_t* ranks, uint8_t* suits, uint8_t n) nogil:
    cdef uint8_t[13] counts_r
    cdef uint8_t[4] counts_s
    cdef uint8_t[5] count_counts
    cdef uint16_t overall = 0
    cdef uint16_t[4] persuit
    cdef uint16_t mxfl = 0
    cdef uint8_t i, r, s
    cdef bint is_flush = False
    cdef bint straight_flush = False
    cdef uint8_t strfl_mx = 0
    cdef uint8_t mxfl_cnt = 0
    
    # Initialize arrays
    memset(<void *>counts_r, 0, sizeof(uint8_t) * 13)
    memset(<void *>counts_s, 0, sizeof(uint8_t) * 4)
    memset(<void *>persuit, 0, sizeof(uint16_t) * 4)
    memset(<void *>count_counts, 0, sizeof(uint8_t) * 5)
    
    cdef uint8_t rank, suit
    cdef uint16_t rank2

    count_counts[0] = n

    # Count ranks and suits
    for i in range(n):
        rank = ranks[i]
        suit = suits[i]

        count_counts[counts_r[rank]] -= 1
        counts_r[rank] += 1
        count_counts[counts_r[rank]] += 1

        counts_s[suit] += 1
        rank2 = 1 << rank
        overall |= rank2
        persuit[suit] |= rank2
    
    # Check for flush and straight flush
    for s in range(4):
        if counts_s[s] >= 5:
            is_flush = True
            mxfl = max_uint16(mxfl, persuit[s])
            mxfl_cnt = counts_s[s]
            for r in range(10):
                if (persuit[s] & straight_masks[r]) == straight_masks[r]:
                    strfl_mx = max_uint8(strfl_mx, <uint8_t>(10 - r))
                    straight_flush = True
                    break
    
    if straight_flush:
        return (9 << 20) | (strfl_mx << 16)
        
    # Quads
    cdef int8_t mx1 = -1
    cdef int8_t mx2 = -1
    cdef int8_t mx3 = -1
    cdef int8_t mx4 = -1
    if count_counts[4] > 0:
        for r in range(13):
            if counts_r[r] == 4:
                mx2 = max_int8(mx2, mx1)
                mx1 = r
            elif counts_r[r] > 0:
                mx2 = r
        return (8 << 20) | (mx1 << 16) | (mx2 << 12)
    
    # Full house
    if count_counts[3] > 0 and count_counts[2]+count_counts[3] > 1:
        for r in range(13):
            if counts_r[r] == 2:
                mx2 = r
            elif counts_r[r] == 3:
                mx2 = max_int8(mx2, mx1)
                mx1 = r 
        return (7 << 20) | (mx1 << 16) | (mx2 << 12)
    
    # Flush
    if is_flush:
        for i in range(mxfl_cnt - 5):
            mxfl &= (mxfl - 1)
        return (6 << 20) | mxfl
    
    # Straight
    for r in range(10):
        if (overall & straight_masks[r]) == straight_masks[r]:
            return (5 << 20) | ((10 - r) << 16)
    
    # Trips
    if count_counts[3] > 0:
        for r in range(13):
            if counts_r[r] == 1 or counts_r[r] == 2:
                mx3 = mx2
                mx2 = r
            elif counts_r[r] == 3:
                mx2 = max_int8(mx2, mx1)
                mx1 = r
        return (4 << 20) | (mx1 << 16) | (mx2 << 12) | (mx3 << 8)
    
    # Two pair
    if count_counts[2] > 1:
        for r in range(13):
            if counts_r[r] == 1:
                mx3 = r
            elif counts_r[r] == 2:
                mx3 = max_int8(mx3, mx2)
                mx2 = mx1
                mx1 = r
        return (3 << 20) | (mx1 << 16) | (mx2 << 12) | (mx3 << 8)
    
    # One pair
    if count_counts[2] > 0:
        for r in range(13):
            if counts_r[r] == 1:
                mx4 = mx3
                mx3 = mx2
                mx2 = r
            elif counts_r[r] == 2:
                mx2 = max_int8(mx2, mx1)
                mx1 = r
        return (2 << 20) | (mx1 << 16) | (mx2 << 12) | (mx3 << 8) | (mx4 << 4)
    
    # High card
    for i in range(n-5):
        overall &= (overall - 1)

    return (1 << 20) | overall

cdef uint32_t _evaluate_mv(uint8_t[::1] ranks, uint8_t[::1] suits):
    cdef Py_ssize_t n_py = ranks.shape[0]
    if n_py < 5 or n_py > 52:
        raise ValueError("Need between 5 and 52 cards to evaluate")
    cdef uint8_t n = <uint8_t>n_py

    cdef uint8_t *r_ptr = &ranks[0]
    cdef uint8_t *s_ptr = &suits[0]

    return evaluate_raw(r_ptr, s_ptr, n)

def evaluate(ranks, suits=None):
    """
    Fast Python-facing API - handles lists, numpy arrays, Card objects.
    Optimized for single-call latency.
    
    Two calling conventions:
    1. evaluate([Card objects])              # eval7-compatible
    2. evaluate([ranks], [suits])            # Original format
    """
    cdef uint8_t n
    cdef uint8_t[52] r_buf
    cdef uint8_t[52] s_buf
    cdef Py_ssize_t i
    cdef uint8_t[::1] rview
    cdef uint8_t[::1] sview
    cdef Card card
    
    # Check if we're using Card objects (eval7-compatible API)
    if suits is None:
        # ranks is actually a list of Card objects
        n = len(ranks)
        if n < 5 or n > 52:
            raise ValueError("Need between 5 and 52 cards to evaluate")
        
        # Extract ranks and suits from Card objects
        for i in range(n):
            if isinstance(ranks[i], Card):
                card = <Card>ranks[i]
                r_buf[i] = card.rank
                s_buf[i] = card.suit
            else:
                raise TypeError(f"Expected Card object, got {type(ranks[i])}")
        
        return evaluate_raw(r_buf, s_buf, n)
    
    # Original API: separate ranks and suits arrays
    else:
        n = len(ranks)
        
        if n < 5 or n > 52:
            raise ValueError("Need between 5 and 52 cards to evaluate")
        
        # Fast path for lists - direct extraction, no numpy overhead
        if isinstance(ranks, list) and isinstance(suits, list):
            for i in range(n):
                r_buf[i] = <uint8_t>(<object>ranks[i])
                s_buf[i] = <uint8_t>(<object>suits[i])
            
            return evaluate_raw(r_buf, s_buf, n)
        
        # Fast path for numpy arrays with memoryview
        else:
            rview = ranks
            sview = suits
            return _evaluate_mv(rview, sview)


def handtype(uint32_t value):
    """
    Return the hand type name for an evaluation result.
    
    Args:
        value: Result from evaluate()
    
    Returns:
        String describing the hand type
    
    Example:
        hand = [Card('As'), Card('Ah'), Card('Ks'), Card('Kh'), Card('Kd')]
        result = evaluate(hand)
        print(handtype(result))  # "Full House"
    """
    cdef uint8_t hand_rank = <uint8_t>(value >> 20)
    
    if hand_rank == 9:
        return "Straight Flush"
    elif hand_rank == 8:
        return "Quads"
    elif hand_rank == 7:
        return "Full House"
    elif hand_rank == 6:
        return "Flush"
    elif hand_rank == 5:
        return "Straight"
    elif hand_rank == 4:
        return "Trips"
    elif hand_rank == 3:
        return "Two Pair"
    elif hand_rank == 2:
        return "Pair"
    elif hand_rank == 1:
        return "High Card"
    else:
        return "Invalid"

class Deck:
    """
    A set of all 52 distinct cards, pregenerated to minimize overhead.
    Also provides a few convenience methods for simple simulations.
    
    Example:
        deck = Deck()
        deck.shuffle()
        hand = deck.deal(7)
        result = evaluate(hand)
    """
    def __init__(self, rng=None):
        """Create a new deck with all 52 cards."""
        self.cards = []
        for rank in RANKS:
            for suit in SUITS:
                card = Card(rank + suit)
                self.cards.append(card)

        if isinstance(rng, Random):
            self.rng = rng
        elif isinstance(rng, int):
            self.rng = Random(rng)
        elif rng is None:
            self.rng = Random()
        else:
            raise TypeError("rng must be an instance of Random, integer, or None")
    
    def __repr__(self):
        return f"Deck({self.cards})"
    
    def __len__(self):
        return len(self.cards)
    
    def __getitem__(self, i):
        return self.cards[i]
    
    def shuffle(self):
        """Randomize the order of the cards in the deck."""
        self.rng.shuffle(self.cards)
    
    def deal(self, n):
        """
        Remove the top n cards from the deck and return them.
        
        Args:
            n: Number of cards to deal
            
        Returns:
            List of n Card objects
            
        Raises:
            ValueError: If there aren't enough cards in the deck
        """
        if n > len(self.cards):
            raise ValueError("Insufficient cards in deck")
        dealt = self.cards[:n]
        del self.cards[:n]
        return dealt
    
    def peek(self, n):
        """
        Return the top n cards from the deck without altering it.
        
        Args:
            n: Number of cards to peek at
            
        Returns:
            List of n Card objects
            
        Raises:
            ValueError: If there aren't enough cards in the deck
        """
        if n > len(self.cards):
            raise ValueError("Insufficient cards in deck")
        return self.cards[:n]
    
    def sample(self, n):
        """
        Return n random cards from the deck. The deck will be unaltered.
        
        Args:
            n: Number of cards to sample
            
        Returns:
            List of n Card objects
            
        Raises:
            ValueError: If there aren't enough cards in the deck
        """
        if n > len(self.cards):
            raise ValueError("Insufficient cards in deck")
        return self.rng.sample(self.cards, n)






# @cython.boundscheck(False)
# @cython.wraparound(False)
# def evaluate(cnp.ndarray[uint8_t, ndim=1] ranks, cnp.ndarray[uint8_t, ndim=1] suits):
#     """Evaluate best 5-card hand from n cards (5-52)"""
#     if ranks.shape[0] < 5 or ranks.shape[0] > 52:
#         raise ValueError("Need between 5 and 52 cards to evaluate")
#     cdef uint8_t n = ranks.shape[0]
    
#     cdef uint8_t[13] counts_r
#     cdef uint8_t[4] counts_s
#     cdef uint8_t[5] count_counts
#     cdef uint16_t overall = 0
#     cdef uint16_t[4] persuit
#     cdef uint16_t mxfl = 0
#     cdef uint8_t i, r, s
#     cdef bint is_flush = False
#     cdef bint straight_flush = False
#     cdef uint8_t strfl_mx = 0
#     cdef uint8_t mxfl_cnt = 0
    
#     # Initialize arrays
#     memset(<void *>counts_r, 0, sizeof(uint8_t) * 13)
#     memset(<void *>counts_s, 0, sizeof(uint8_t) * 4)
#     memset(<void *>persuit, 0, sizeof(uint16_t) * 4)
#     memset(<void *>count_counts, 0, sizeof(uint8_t) * 5)
    
#     cdef uint8_t rank, suit
#     cdef uint16_t rank2

#     count_counts[0] = n

#     # Count ranks and suits
#     for i in range(n):
#         rank = ranks[i]
#         suit = suits[i]

#         count_counts[counts_r[rank]] -= 1
#         counts_r[rank] += 1
#         count_counts[counts_r[rank]] += 1

#         counts_s[suit] += 1
#         rank2 = 1 << rank
#         overall |= rank2
#         persuit[suit] |= rank2
    
#     # Check for flush and straight flush
#     for s in range(4):
#         if counts_s[s] >= 5:
#             is_flush = True
#             mxfl = max_uint16(mxfl, persuit[s])
#             mxfl_cnt = counts_s[s]
#             for r in range(10):
#                 if (persuit[s] & straight_masks[r]) == straight_masks[r]:
#                     strfl_mx = max_uint8(strfl_mx, <uint8_t>(10 - r))
#                     straight_flush = True
#                     break
    
#     if straight_flush:
#         return (9 << 20) | (strfl_mx << 16)
        
#     # Quads
#     cdef int8_t mx1 = -1
#     cdef int8_t mx2 = -1
#     cdef int8_t mx3 = -1
#     cdef int8_t mx4 = -1
#     if count_counts[4] > 0:
#         for r in range(13):
#             if counts_r[r] == 4:
#                 mx2 = max_int8(mx2, mx1)
#                 mx1 = r
#             elif counts_r[r] > 0:
#                 mx2 = r
#         return (8 << 20) | (mx1 << 16) | (mx2 << 12)
    
#     # Full house
#     if count_counts[3] > 0 and count_counts[2]+count_counts[3] > 1:
#         for r in range(13):
#             if counts_r[r] == 2:
#                 mx2 = r
#             elif counts_r[r] == 3:
#                 mx2 = max_int8(mx2, mx1)
#                 mx1 = r 
#         return (7 << 20) | (mx1 << 16) | (mx2 << 12)
    
#     # Flush
#     if is_flush:
#         for i in range(mxfl_cnt - 5):
#             mxfl &= (mxfl - 1)
#         return (6 << 20) | mxfl
    
#     # Straight
#     for r in range(10):
#         if (overall & straight_masks[r]) == straight_masks[r]:
#             return (5 << 20) | ((10 - r) << 16)
    
#     # Trips
#     if count_counts[3] > 0:
#         for r in range(13):
#             if counts_r[r] == 1 or counts_r[r] == 2:
#                 mx3 = mx2
#                 mx2 = r
#             elif counts_r[r] == 3:
#                 mx2 = max_int8(mx2, mx1)
#                 mx1 = r
#         return (4 << 20) | (mx1 << 16) | (mx2 << 12) | (mx3 << 8)
    
#     # Two pair
#     if count_counts[2] > 1:
#         for r in range(13):
#             if counts_r[r] == 1:
#                 mx3 = r
#             elif counts_r[r] == 2:
#                 mx3 = max_int8(mx3, mx2)
#                 mx2 = mx1
#                 mx1 = r
#         return (3 << 20) | (mx1 << 16) | (mx2 << 12) | (mx3 << 8)
    
#     # One pair
#     if count_counts[2] > 0:
#         for r in range(13):
#             if counts_r[r] == 1:
#                 mx4 = mx3
#                 mx3 = mx2
#                 mx2 = r
#             elif counts_r[r] == 2:
#                 mx2 = max_int8(mx2, mx1)
#                 mx1 = r
#         return (2 << 20) | (mx1 << 16) | (mx2 << 12) | (mx3 << 8) | (mx4 << 4)
    
#     # High card
#     for i in range(n-5):
#         overall &= (overall - 1)

#     return (1 << 20) | overall