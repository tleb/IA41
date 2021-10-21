% position(A, X, Y)
% Defines every position and associatiates it with its coordinates.
position(0, 0, 0).
position(1, 1, 0).
position(2, 2, 0).
position(3, 0, 1).
position(4, 1, 1).
position(5, 2, 1).
position(6, 0, 2).
position(7, 1, 2).
position(8, 2, 2).


% Defines the two possible players
player(pw).  % player white
player(pb).  % player black


opponent(pw, pb).
opponent(pb, pw).


% B = A >= 0 ? A : -A
abs(A, B) :- A >= 0, B is A.
abs(A, B) :- A < 0, B is -A.


% C = A <= B ? A : B
min(A, B, C) :- A >= B, C is B.
min(A, B, C) :- A < B, C is A.


% C = A >= B ? A : B
max(A, B, C) :- A >= B, C is A.
max(A, B, C) :- A < B, C is B.


% calculates the distance between A and B (two positions)
distance(A, B, Distance) :-
	position(A, AX, AY),
	position(B, BX, BY),
	abs(BX-AX, DiffX),
	abs(BY-AY, DiffY),
	Distance is DiffX+DiffY.


% ownedByPlayer(Player, Tower)
ownedByPlayer(P, [P|_]).


countOccupiedPositions([], 0).
countOccupiedPositions([[_|_]|R], N) :- countOccupiedPositions(R, N2), N is N2+1.
countOccupiedPositions([[]|R], N) :- countOccupiedPositions(R, N).


countPlayerPositions(_, [], 0).
countPlayerPositions(P, [[P|_]|R], N) :- countPlayerPositions(P, R, N2), N is N2+1.
countPlayerPositions(P, [[P2|_]|R], N) :- P \= P2, countPlayerPositions(P, R, N).
countPlayerPositions(P, [[]|R], N) :- countPlayerPositions(P, R, N).


score(Player, Towers, Score) :-
	countPlayerPositions(Player, Towers, N),
	countOccupiedPositions(Towers, N2),
	Score is N/N2.


% nthTower(Towers, N, Tower)
% Tower is the N-th tower in Towers
nthTower([T|_], 0, T).
nthTower([_|R], N, T) :- N \= 0, M is N-1, nthTower(R, M, T).


playerWon(Player, Towers) :- countPlayerPositions(Player, Towers, N), countOccupiedPositions(Towers, N2), N = N2.


gameFinished(Towers) :- player(Player), playerWon(Player, Towers), !.


% takeHead(N, Source, Destination)
% Destination = Source[:N]
takeHead(0, _, []).
takeHead(_, [], []).
takeHead(N, [A|RS], [B|RD]) :- N > 0, A = B, M is N-1, takeHead(M, RS, RD).


% removeHead(N, Source, Destination)
% Destination = Source[N:]
removeHead(0, L, L).
removeHead(N, [_|RS], D) :- N > 0, M is N-1, removeHead(M, RS, D).


% movePieces(StartPosition, EndPosition, CurrentPosition, PiecesToMove, InitialTowers, FinalTowers)
% FinalTowers = InitialTowers except for StartPosition which should have
% PiecesToMove removed and EndPosition which should have PiecesToMove prepended
movePieces(_, _, _, _, [], []).
movePieces(SP, EP, CP, PTM, [IT|RIT], [FT|RFT]) :-
	movePiecesHelper(SP, EP, CP, PTM, IT, FT),
	NP is CP+1,
	movePieces(SP, EP, NP, PTM, RIT, RFT).


% movePiecesHelper(StartPosition, EndPosition, CurrentPosition, PiecesToMove, InitialTower, FinalTower)
movePiecesHelper(SP, EP, SP, PTM, IT, FT) :- SP \= EP, length(PTM, Length), removeHead(Length, IT, FT).
movePiecesHelper(SP, EP, EP, PTM, IT, FT) :- SP \= EP, append(PTM, IT, FT).
movePiecesHelper(SP, EP, CP, _, IT, IT) :- CP \= SP, CP \= EP.


% next state
ns([InitialPlayer, InitialTowers], [FinalPlayer, FinalTowers]) :-
	position(StartPosition, _, _),
	nthTower(InitialTowers, StartPosition, StartTower),
	ownedByPlayer(InitialPlayer, StartTower),
	length(StartTower, TowerSize),
	min(TowerSize, 3, MaxPieceCountToMove),
	between(1, MaxPieceCountToMove, PieceCountToMove),
	distance(StartPosition, EndPosition, PieceCountToMove),
	takeHead(PieceCountToMove, StartTower, PiecesToMove),
	opponent(InitialPlayer, FinalPlayer),
	movePieces(StartPosition, EndPosition, 0, PiecesToMove, InitialTowers, FinalTowers).


% maxValue(State, P, Value, Alpha, Beta)
maxValue([Player, Towers], 0, Value, _, _, _) :- score(Player, Towers, Value), !.
maxValue([Player, Towers], P, Value, _, _, _) :- P \= 0, gameFinished(Towers), score(Player, Towers, Value), !.
maxValue([Player, Towers], P, FinalValue, FinalState, Alpha, Beta) :-
	P \= 0,
	findall(Successor, ns([Player, Towers], Successor), AllSuccessors),
	findMaxState(AllSuccessors, P, FinalValue, FinalState, Alpha, Beta).

findMaxState([], _, -1.0Inf, [], _, _) :- !.
findMaxState([Successor|Tail], P, FinalValue, FinalState, Alpha, Beta) :-
	Q is P-1,
	minValue(Successor, Q, CurrentValue, _, Alpha, Beta),
	(
		CurrentValue >= Beta
		->
		FinalValue = CurrentValue,
		FinalState = Successor
		;
		NewAlpha is max(Alpha, CurrentValue),
		findMaxState(Tail, P, TailValue, TailState, NewAlpha, Beta),
		best([CurrentValue, Successor], [TailValue, TailState], [FinalValue, FinalState])
	).

best([Value1, State1], [Value2, _], [Value1, State1]) :- Value1 >= Value2, !.
best([Value1, _], [Value2, State2], [Value2, State2]) :- Value1 < Value2.



% minValue(State, P, Value, Alpha, Beta)
minValue([Player, Towers], 0, Value, [], _, _) :- opponent(Player, OppositePlayer), score(OppositePlayer, Towers, Value), !.
minValue([Player, Towers], P, Value, [], _, _) :- P \= 0, gameFinished(Towers), opponent(Player, OppositePlayer), score(OppositePlayer, Towers, Value), !.
minValue([Player, Towers], P, Value, FinalState, Alpha, Beta) :-
	P \= 0,
	findall(Successor, ns([Player, Towers], Successor), AllSuccessors),
	findMinState(AllSuccessors, P, Value, FinalState, Alpha, Beta).

findMinState([], _, 1.0Inf, [], _, _) :- !.
findMinState([Successor|Tail], P, FinalValue, FinalState, Alpha, Beta) :-
	Q is P-1,
	maxValue(Successor, Q, CurrentValue, _, Alpha, Beta),
	(
		Alpha >= CurrentValue
		->
		FinalValue = CurrentValue,
		FinalState = Successor
		;
		NewBeta is min(Beta, CurrentValue),
		findMinState(Tail, P, TailValue, TailState, Alpha, NewBeta),
		worst([CurrentValue, Successor], [TailValue, TailState], [FinalValue, FinalState])
	).

worst([Value1, State1], [Value2, _], [Value1, State1]) :- Value2 >= Value1, !.
worst([Value1, _], [Value2, State2], [Value2, State2]) :- Value1 > Value2.


main() :-
	current_prolog_flag(argv, Argv),
	Argv=[Atom1,Atom2|_],
	atom_to_term(Atom1, Depth, []),
	atom_to_term(Atom2, State, []),
	maxValue(State, Depth, _, NextState, -1.0Inf, 1.0Inf),
	write(NextState), !.


/*
Fonction MinMaxPL(e, p) retourne une action
	v <- MaxValue(e, p, -inf, +inf)
	Retourner action dont l'état successeur de e vaut v
Fin fonction

Fonction  MaxValue(e, p, alpha, beta)
	Si (e est terminal ou p = 0) Alors Retourner eval(e)

	v <- -∞
	Pour chaque successeur s de e Faire
		v <- max(v, MinValue(s, p-1, alpha, beta))
		Si v >= beta Alors Retourner v
		alpha <- max(alpha, v)
	Fin pour

	Retourner v
Fin fonction

Fonction MinValue(e, p, alpha, beta)
	Si (e est terminal ou p = 0) Alors Retourner eval(e)

	v <- +∞
	Pour chaque successeur s de e Faire
		v <- min(v, MaxValue(s, p-1))
		Si v <= alpha Alors Retourner v
		beta <- min(beta, v)
	Fin pour

	Retourner v
Fin fonction
*/
