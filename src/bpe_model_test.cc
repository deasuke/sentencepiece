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

#include "bpe_model.h"
#include "testharness.h"

namespace sentencepiece {
namespace bpe {
namespace {

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

TEST(BPEModelTest, EncodeTest) {
  ModelProto model_proto = MakeBaseModelProto();

  AddPiece(&model_proto, "ab", -0.1);
  AddPiece(&model_proto, "cd", -0.2);
  AddPiece(&model_proto, "abc", -0.3);
  AddPiece(&model_proto, "a", -0.4);
  AddPiece(&model_proto, "b", -0.5);
  AddPiece(&model_proto, "c", -0.6);
  AddPiece(&model_proto, "d", -0.7);

  const Model model(model_proto);

  std::vector<std::pair<StringPiece, int>> result;

  result = model.Encode("");
  EXPECT_TRUE(result.empty());

  result = model.Encode("abc");
  EXPECT_EQ(1, result.size());
  EXPECT_EQ("abc", result[0].first);

  result = model.Encode("AB");
  EXPECT_EQ(2, result.size());
  EXPECT_EQ("A", result[0].first);
  EXPECT_EQ("B", result[1].first);

  result = model.Encode("abcd");
  EXPECT_EQ(2, result.size());
  EXPECT_EQ("ab", result[0].first);
  EXPECT_EQ("cd", result[1].first);

  result = model.Encode("abcc");
  EXPECT_EQ(2, result.size());
  EXPECT_EQ("abc", result[0].first);
  EXPECT_EQ("c", result[1].first);

  result = model.Encode("xabcabaabcdd");
  EXPECT_EQ(7, result.size());
  EXPECT_EQ("x", result[0].first);
  EXPECT_EQ("abc", result[1].first);
  EXPECT_EQ("ab", result[2].first);
  EXPECT_EQ("a", result[3].first);
  EXPECT_EQ("ab", result[4].first);
  EXPECT_EQ("cd", result[5].first);
  EXPECT_EQ("d", result[6].first);

  // all unknown.
  result = model.Encode("xyz東京");
  EXPECT_EQ(5, result.size());
  EXPECT_EQ("x", result[0].first);
  EXPECT_EQ("y", result[1].first);
  EXPECT_EQ("z", result[2].first);
  EXPECT_EQ("東", result[3].first);
  EXPECT_EQ("京", result[4].first);
}

TEST(BPEModelTest, EncodeAmbiguousTest) {
  ModelProto model_proto = MakeBaseModelProto();

  AddPiece(&model_proto, "aa", -0.1);
  AddPiece(&model_proto, "bb", -0.2);
  AddPiece(&model_proto, "ab", -0.3);
  AddPiece(&model_proto, "a", -0.4);
  AddPiece(&model_proto, "b", -0.5);

  const Model model(model_proto);

  std::vector<std::pair<StringPiece, int>> result;

  // leftmost symbols are merged first.
  result = model.Encode("aaa");
  EXPECT_EQ(2, result.size());
  EXPECT_EQ("aa", result[0].first);
  EXPECT_EQ("a", result[1].first);

  // "bb" is replaced earlier than "ab".
  result = model.Encode("aabb");
  EXPECT_EQ(2, result.size());
  EXPECT_EQ("aa", result[0].first);
  EXPECT_EQ("bb", result[1].first);

  // "bb" is replaced earlier than "ab".
  result = model.Encode("aaabbb");
  EXPECT_EQ(4, result.size());
  EXPECT_EQ("aa", result[0].first);
  EXPECT_EQ("a", result[1].first);
  EXPECT_EQ("bb", result[2].first);
  EXPECT_EQ("b", result[3].first);

  result = model.Encode("aaaba");
  EXPECT_EQ(3, result.size());
  EXPECT_EQ("aa", result[0].first);
  EXPECT_EQ("ab", result[1].first);
  EXPECT_EQ("a", result[2].first);
}

}  // namespace
}  // namespace bpe
}  // namespace sentencepiece
