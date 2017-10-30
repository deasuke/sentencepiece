// Copyright 2016 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.!

#include "unigram_model.h"

#include <fstream>
#include <unordered_map>
#include <unordered_set>

#include "sentencepiece_model.pb.h"
#include "testharness.h"
#include "util.h"

namespace sentencepiece {
namespace unigram {

TEST(LatticeTest, SetSentenceTest) {
  Lattice lattice;

  EXPECT_EQ(0, lattice.size());
  EXPECT_EQ(0, lattice.utf8_size());

  lattice.SetSentence("test");
  EXPECT_EQ(4, lattice.size());
  EXPECT_EQ(4, lattice.utf8_size());
  EXPECT_STREQ("test", lattice.sentence());
  EXPECT_STREQ("test", lattice.surface(0));
  EXPECT_STREQ("est", lattice.surface(1));
  EXPECT_STREQ("st", lattice.surface(2));
  EXPECT_STREQ("t", lattice.surface(3));

  Lattice::Node *bos = lattice.bos_node();
  Lattice::Node *eos = lattice.eos_node();

  EXPECT_EQ(-1, bos->id);
  EXPECT_EQ(-1, eos->id);
  EXPECT_EQ(bos, lattice.end_nodes(0).front());
  EXPECT_EQ(eos, lattice.begin_nodes(4).front());

  lattice.SetSentence("テストab");
  EXPECT_EQ(5, lattice.size());
  EXPECT_EQ(11, lattice.utf8_size());
  EXPECT_STREQ("テストab", lattice.sentence());
  EXPECT_STREQ("テストab", lattice.surface(0));
  EXPECT_STREQ("ストab", lattice.surface(1));
  EXPECT_STREQ("トab", lattice.surface(2));
  EXPECT_STREQ("ab", lattice.surface(3));
  EXPECT_STREQ("b", lattice.surface(4));

  lattice.Clear();
  EXPECT_EQ(0, lattice.size());
  EXPECT_EQ(0, lattice.utf8_size());
}

TEST(LatticeTest, InsertTest) {
  Lattice lattice;
  lattice.SetSentence("ABあい");

  Lattice::Node *node[7];
  node[0] = lattice.Insert(0, 1);
  node[1] = lattice.Insert(1, 1);
  node[2] = lattice.Insert(2, 1);
  node[3] = lattice.Insert(3, 1);
  node[4] = lattice.Insert(0, 2);
  node[5] = lattice.Insert(1, 2);
  node[6] = lattice.Insert(2, 2);

  EXPECT_EQ("A", node[0]->piece);
  EXPECT_EQ("B", node[1]->piece);
  EXPECT_EQ("あ", node[2]->piece);
  EXPECT_EQ("い", node[3]->piece);
  EXPECT_EQ("AB", node[4]->piece);
  EXPECT_EQ("Bあ", node[5]->piece);
  EXPECT_EQ("あい", node[6]->piece);

  EXPECT_EQ("A", node[0]->piece);
  EXPECT_EQ("B", node[1]->piece);
  EXPECT_EQ("あ", node[2]->piece);
  EXPECT_EQ("い", node[3]->piece);
  EXPECT_EQ("AB", node[4]->piece);
  EXPECT_EQ("Bあ", node[5]->piece);
  EXPECT_EQ("あい", node[6]->piece);

  EXPECT_EQ(0, node[0]->pos);
  EXPECT_EQ(1, node[1]->pos);
  EXPECT_EQ(2, node[2]->pos);
  EXPECT_EQ(3, node[3]->pos);
  EXPECT_EQ(0, node[4]->pos);
  EXPECT_EQ(1, node[5]->pos);
  EXPECT_EQ(2, node[6]->pos);

  EXPECT_EQ(1, node[0]->length);
  EXPECT_EQ(1, node[1]->length);
  EXPECT_EQ(1, node[2]->length);
  EXPECT_EQ(1, node[3]->length);
  EXPECT_EQ(2, node[4]->length);
  EXPECT_EQ(2, node[5]->length);
  EXPECT_EQ(2, node[6]->length);

  EXPECT_EQ(0, lattice.bos_node()->node_id);
  EXPECT_EQ(1, lattice.eos_node()->node_id);
  EXPECT_EQ(2, node[0]->node_id);
  EXPECT_EQ(3, node[1]->node_id);
  EXPECT_EQ(4, node[2]->node_id);
  EXPECT_EQ(5, node[3]->node_id);
  EXPECT_EQ(6, node[4]->node_id);
  EXPECT_EQ(7, node[5]->node_id);
  EXPECT_EQ(8, node[6]->node_id);

  EXPECT_EQ(2, lattice.begin_nodes(0).size());
  EXPECT_EQ(2, lattice.begin_nodes(1).size());
  EXPECT_EQ(2, lattice.begin_nodes(2).size());
  EXPECT_EQ(1, lattice.begin_nodes(3).size());
  EXPECT_EQ(1, lattice.begin_nodes(4).size());  // EOS

  EXPECT_EQ(1, lattice.end_nodes(0).size());  // BOS
  EXPECT_EQ(1, lattice.end_nodes(1).size());
  EXPECT_EQ(2, lattice.end_nodes(2).size());
  EXPECT_EQ(2, lattice.end_nodes(3).size());
  EXPECT_EQ(2, lattice.end_nodes(4).size());

  EXPECT_EQ(node[0], lattice.begin_nodes(0)[0]);
  EXPECT_EQ(node[4], lattice.begin_nodes(0)[1]);
  EXPECT_EQ(node[1], lattice.begin_nodes(1)[0]);
  EXPECT_EQ(node[5], lattice.begin_nodes(1)[1]);
  EXPECT_EQ(node[2], lattice.begin_nodes(2)[0]);
  EXPECT_EQ(node[6], lattice.begin_nodes(2)[1]);
  EXPECT_EQ(node[3], lattice.begin_nodes(3)[0]);
  EXPECT_EQ(lattice.eos_node(), lattice.begin_nodes(4)[0]);

  EXPECT_EQ(lattice.bos_node(), lattice.end_nodes(0)[0]);
  EXPECT_EQ(node[0], lattice.end_nodes(1)[0]);
  EXPECT_EQ(node[1], lattice.end_nodes(2)[0]);
  EXPECT_EQ(node[4], lattice.end_nodes(2)[1]);
  EXPECT_EQ(node[2], lattice.end_nodes(3)[0]);
  EXPECT_EQ(node[5], lattice.end_nodes(3)[1]);
  EXPECT_EQ(node[3], lattice.end_nodes(4)[0]);
  EXPECT_EQ(node[6], lattice.end_nodes(4)[1]);
}

TEST(LatticeTest, ViterbiFromIncompleteLatticeTest) {
  Lattice lattice;
  lattice.SetSentence("ABC");
  EXPECT_DEATH(lattice.Viterbi());

  // Still incomplete
  lattice.Insert(0, 1);
  EXPECT_DEATH(lattice.Viterbi());

  lattice.Insert(1, 1);
  lattice.Insert(2, 1);
  lattice.Viterbi();
}

std::string GetTokenized(const std::vector<Lattice::Node *> &nodes) {
  std::vector<std::string> tokens;
  for (auto *node : nodes) {
    tokens.push_back(node->piece.to_string());
  }
  return string_util::Join(tokens, " ");
}

void InsertWithScore(Lattice *lattice, int pos, int length, float score) {
  lattice->Insert(pos, length)->score = score;
}

void InsertWithScoreAndId(Lattice *lattice, int pos, int length, float score,
                          int id) {
  auto *node = lattice->Insert(pos, length);
  node->score = score;
  node->id = id;
}

TEST(LatticeTest, ViterbiTest) {
  Lattice lattice;
  lattice.SetSentence("ABC");

  InsertWithScore(&lattice, 0, 1, 0.0);  // A
  InsertWithScore(&lattice, 1, 1, 0.0);  // B
  InsertWithScore(&lattice, 2, 1, 0.0);  // C
  EXPECT_EQ("A B C", GetTokenized(lattice.Viterbi()));

  InsertWithScore(&lattice, 0, 2, 2.0);  // AB
  EXPECT_EQ("AB C", GetTokenized(lattice.Viterbi()));

  InsertWithScore(&lattice, 1, 2, 5.0);  // BC
  EXPECT_EQ("A BC", GetTokenized(lattice.Viterbi()));

  InsertWithScore(&lattice, 0, 3, 10.0);  // ABC
  EXPECT_EQ("ABC", GetTokenized(lattice.Viterbi()));
}

TEST(LatticeTest, NBestTest) {
  Lattice lattice;
  lattice.SetSentence("ABC");

  InsertWithScore(&lattice, 0, 1, 0.0);   // A
  InsertWithScore(&lattice, 1, 1, 0.0);   // B
  InsertWithScore(&lattice, 2, 1, 0.0);   // C
  InsertWithScore(&lattice, 0, 2, 2.0);   // AB
  InsertWithScore(&lattice, 1, 2, 5.0);   // BC
  InsertWithScore(&lattice, 0, 3, 10.0);  // ABC

  auto nbests = lattice.NBest(10);
  EXPECT_EQ(4, nbests.size());

  EXPECT_EQ("ABC", GetTokenized(nbests[0]));
  EXPECT_EQ("A BC", GetTokenized(nbests[1]));
  EXPECT_EQ("AB C", GetTokenized(nbests[2]));
  EXPECT_EQ("A B C", GetTokenized(nbests[3]));
}

TEST(LatticeTest, PopulateMarginalTest) {
  Lattice lattice;
  lattice.SetSentence("ABC");

  InsertWithScoreAndId(&lattice, 0, 1, 1.0, 0);  // A
  InsertWithScoreAndId(&lattice, 1, 1, 1.2, 1);  // B
  InsertWithScoreAndId(&lattice, 2, 1, 2.5, 2);  // C
  InsertWithScoreAndId(&lattice, 0, 2, 3.0, 3);  // AB
  InsertWithScoreAndId(&lattice, 1, 2, 4.0, 4);  // BC
  InsertWithScoreAndId(&lattice, 0, 3, 2.0, 5);  // ABC

  std::vector<float> probs(6, 0.0);

  // Expand all paths:
  // A B C : exp(1.0 + 1.2 + 2.5) => path1
  // AB  C : exp(3.0 + 2.5)       => path2
  // A  BC : exp(1.0 + 4.0)       => path3
  // ABC   : exp(2.0)             => path4
  const float p1 = exp(1.0 + 1.2 + 2.5);
  const float p2 = exp(3.0 + 2.5);
  const float p3 = exp(1.0 + 4.0);
  const float p4 = exp(2.0);
  const float Z = p1 + p2 + p3 + p4;

  const float logZ = lattice.PopulateMarginal(1.0, &probs);

  EXPECT_NEAR((p1 + p3) / Z, probs[0], 0.001);  // A
  EXPECT_NEAR(p1 / Z, probs[1], 0.001);         // B
  EXPECT_NEAR((p1 + p2) / Z, probs[2], 0.001);  // C
  EXPECT_NEAR(p2 / Z, probs[3], 0.001);         // AB
  EXPECT_NEAR(p3 / Z, probs[4], 0.001);         // BC
  EXPECT_NEAR(p4 / Z, probs[5], 0.001);         // ABC
  EXPECT_NEAR(log(Z), logZ, 0.001);
}

ModelProto MakeBaseModelProto() {
  ModelProto model_proto;
  auto *sp2 = model_proto.add_pieces();
  auto *sp3 = model_proto.add_pieces();
  auto *sp1 = model_proto.add_pieces();

  sp1->set_type(ModelProto::SentencePiece::UNKNOWN);
  sp1->set_piece("<unk>");
  sp2->set_type(ModelProto::SentencePiece::CONTROL);
  sp2->set_piece("<s>");
  sp3->set_type(ModelProto::SentencePiece::CONTROL);
  sp3->set_piece("</s>");

  return model_proto;
}

void AddPiece(ModelProto *model_proto, const std::string &piece,
              float score = 0.0) {
  auto *sp = model_proto->add_pieces();
  sp->set_piece(piece);
  sp->set_score(score);
}

TEST(UnigramModelTest, SetUnigramModelTest) {
  ModelProto model_proto = MakeBaseModelProto();

  AddPiece(&model_proto, "a");
  AddPiece(&model_proto, "b");
  AddPiece(&model_proto, "c");
  AddPiece(&model_proto, "d");

  const Model model(model_proto);
  EXPECT_EQ(model_proto.SerializeAsString(),
            model.model_proto().SerializeAsString());
}

TEST(UnigramModelTest, PieceToIdTest) {
  ModelProto model_proto = MakeBaseModelProto();

  AddPiece(&model_proto, "a", 0.1);
  AddPiece(&model_proto, "b", 0.2);
  AddPiece(&model_proto, "c", 0.3);
  AddPiece(&model_proto, "d", 0.4);

  const Model model(model_proto);
  EXPECT_EQ(model_proto.SerializeAsString(),
            model.model_proto().SerializeAsString());

  EXPECT_NEAR(0.1, model.min_score(), 0.001);

  EXPECT_EQ(2, model.PieceToId("<unk>"));
  EXPECT_EQ(0, model.PieceToId("<s>"));
  EXPECT_EQ(1, model.PieceToId("</s>"));
  EXPECT_EQ(3, model.PieceToId("a"));
  EXPECT_EQ(4, model.PieceToId("b"));
  EXPECT_EQ(5, model.PieceToId("c"));
  EXPECT_EQ(6, model.PieceToId("d"));
  EXPECT_EQ(2, model.PieceToId("e"));  // unk
  EXPECT_EQ(2, model.PieceToId(""));   // unk

  EXPECT_EQ("<unk>", model.IdToPiece(2));
  EXPECT_EQ("<s>", model.IdToPiece(0));
  EXPECT_EQ("</s>", model.IdToPiece(1));
  EXPECT_EQ("a", model.IdToPiece(3));
  EXPECT_EQ("b", model.IdToPiece(4));
  EXPECT_EQ("c", model.IdToPiece(5));
  EXPECT_EQ("d", model.IdToPiece(6));

  EXPECT_TRUE(model.IsUnknown(2));
  EXPECT_FALSE(model.IsUnknown(0));
  EXPECT_FALSE(model.IsUnknown(1));
  EXPECT_FALSE(model.IsUnknown(3));
  EXPECT_FALSE(model.IsUnknown(4));
  EXPECT_FALSE(model.IsUnknown(5));
  EXPECT_FALSE(model.IsUnknown(6));

  EXPECT_FALSE(model.IsControl(2));
  EXPECT_TRUE(model.IsControl(0));
  EXPECT_TRUE(model.IsControl(1));
  EXPECT_FALSE(model.IsControl(3));
  EXPECT_FALSE(model.IsControl(4));
  EXPECT_FALSE(model.IsControl(5));
  EXPECT_FALSE(model.IsControl(6));

  EXPECT_NEAR(0, model.GetScore(0), 0.0001);
  EXPECT_NEAR(0, model.GetScore(1), 0.0001);
  EXPECT_NEAR(0, model.GetScore(2), 0.0001);
  EXPECT_NEAR(0.1, model.GetScore(3), 0.0001);
  EXPECT_NEAR(0.2, model.GetScore(4), 0.0001);
  EXPECT_NEAR(0.3, model.GetScore(5), 0.0001);
  EXPECT_NEAR(0.4, model.GetScore(6), 0.0001);

  EXPECT_TRUE(model.Encode("").empty());
}

TEST(UnigramModelTest, PopulateNodesAllUnknownsTest) {
  ModelProto model_proto = MakeBaseModelProto();
  AddPiece(&model_proto, "x");
  const Model model(model_proto);

  Lattice lattice;
  lattice.SetSentence("abc");
  model.PopulateNodes(&lattice);

  EXPECT_EQ(1, lattice.begin_nodes(0).size());
  EXPECT_EQ(1, lattice.begin_nodes(1).size());
  EXPECT_EQ(1, lattice.begin_nodes(2).size());

  EXPECT_EQ(2, lattice.begin_nodes(0)[0]->id);
  EXPECT_EQ(2, lattice.begin_nodes(1)[0]->id);
  EXPECT_EQ(2, lattice.begin_nodes(2)[0]->id);
}

TEST(UnigramModelTest, PopulateNodesTest) {
  ModelProto model_proto = MakeBaseModelProto();

  AddPiece(&model_proto, "a", 0.1);   // 3
  AddPiece(&model_proto, "b", 0.2);   // 4
  AddPiece(&model_proto, "ab", 0.3);  // 5
  AddPiece(&model_proto, "bc", 0.4);  // 6

  const Model model(model_proto);

  Lattice lattice;
  lattice.SetSentence("abc");

  model.PopulateNodes(&lattice);

  EXPECT_EQ(2, lattice.begin_nodes(0).size());  // a,ab
  EXPECT_EQ(2, lattice.begin_nodes(1).size());  // b,bc
  EXPECT_EQ(1, lattice.begin_nodes(2).size());  // c(unk)

  EXPECT_EQ(3, lattice.begin_nodes(0)[0]->id);
  EXPECT_EQ(5, lattice.begin_nodes(0)[1]->id);
  EXPECT_EQ(4, lattice.begin_nodes(1)[0]->id);
  EXPECT_EQ(6, lattice.begin_nodes(1)[1]->id);
  EXPECT_EQ(2, lattice.begin_nodes(2)[0]->id);

  EXPECT_NEAR(0.1, lattice.begin_nodes(0)[0]->score, 0.001);
  EXPECT_NEAR(0.3, lattice.begin_nodes(0)[1]->score, 0.001);
  EXPECT_NEAR(0.2, lattice.begin_nodes(1)[0]->score, 0.001);
  EXPECT_NEAR(0.4, lattice.begin_nodes(1)[1]->score, 0.001);
}
}  // namespace unigram
}  // namespace sentencepiece
