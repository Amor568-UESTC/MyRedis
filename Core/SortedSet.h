#pragma once

#include<map>
#include<set>
#include<vector>
#include<unordered_map>

#include"String.h"
#include"Helper.h"

namespace myredis
{

class SortedSet
{
public:
    using Members=std::set<String>;
    using Score2Members=std::map<double,Members>;
    using Member2Score=std::unordered_map<String,double,
                                          my_hash,
                                          std::equal_to<String>>;

private:
    Score2Members scores_;
    Member2Score members_;

public:
    Member2Score::iterator FindMember(const String& member);
    Member2Score::const_iterator begin() const { return members_.begin(); };
    Member2Score::iterator begin() { return members_.begin(); }
    Member2Score::const_iterator end() const { return members_.end(); }
    Member2Score::iterator end() { return members_.end(); }

    void AddMember(const String& member,double score);
    double UpdateMember(Member2Score::iterator& itMem,double delta);
    int Rank(const String& member) const;
    int RevRank(const String& member) const;
    bool DelMember(const String& member);

    Member2Score::value_type GetMemberByRank(size_t rank) const;
    std::vector<Member2Score::value_type> RangeByRank(long start,long end) const;
    std::vector<Member2Score::value_type> RangeByScore(double minScore,double MaxScore);
    size_t Size() const;
};

}