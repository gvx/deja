from __future__ import print_function
import os
import networkx as nx
import matplotlib.pyplot as plt

from collections import defaultdict

def getnames():
	return [name for name in os.listdir('.') if name[-2:] in ('.c', '.h')]

def getdeps(name):
	with open(name) as f:
		for line in f:
			if line.startswith('#include "'):
				yield (name, line.rstrip()[10:-1])

def buildgraph(names):
	G = nx.DiGraph()
	for name in names:
		G.add_edges_from(getdeps(name))
	return G

def follow(G, n, counter, found):
	found.add(n)
	for outgoing in G.successors(n):
		counter[outgoing] += 1
		if outgoing not in found:
			follow(G, outgoing, counter, found)

def doubledeps(G):
	indirect = {}
	for n in G.nodes():
		indirect[n] = defaultdict(int)
		follow(G, n, indirect[n], set())
	return indirect

def allfrom(indirect):
	for x in indirect:
		for y in indirect[x]:
			yield x, y, indirect[x][y]

def prettylist(indirect):
	return sorted(allfrom(indirect), key=lambda x: x[2])

def followdeps(G, n):
	visited = set()
	for out in G.successors(n):
		for x in followdeps_(G, out, visited):
			yield x

def followdeps_(G, n, v):
	if n in v:
		return
	for out in G.successors(n):
		yield out
		v.add(out)
		for x in followdeps_(G, out, v):
			yield x

def removable(G):
	for n in G.nodes():
		yield n, set(G.successors(n)) & set(followdeps(G, n))

def prettylist2(x):
	return sorted(x, key=lambda x: len(x[1]))

G = buildgraph(getnames())

# also interesting, but the second one is more useful
#for line in prettylist(doubledeps(G)):
#	print line

for name, deps in prettylist2(removable(G)):
	if deps:
		print(name, "has a superfluous dependency on", ', '.join(deps))

nx.draw(G)
plt.show()
