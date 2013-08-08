#ifndef FB_NODE_H_
#define FB_NODE_H_

#include "NodeBase.h"

#include <string.h>
#include <json_spirit.h>
#include <vector>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/mem_fun.hpp>

#include <boost/function.hpp>
#include <boost/date_time.hpp>
#include <time.h>

using namespace std;

typedef std::time_t facebook_time;

namespace Facebook {

	class FBNode;

	class FBDataSource {

	public:
		virtual void loadField(FBNode * parent, string nodeQuery, string interpretAs, boost::function<void(FBNode*)> loadCompleteCallback) = 0;
		virtual void load(FBNode * parent, string id, string edge) = 0;
		virtual void loadQuery(FBNode * parent, string nodeQuery, string interpretAs, boost::function<void(FBNode*)> loadCompleteCallback) = 0;
		static FBDataSource * instance;


	};


	struct Edge {

		long RowNumber;

		string Type;		
		FBNode * Node;
		
		string Value;
		json_spirit::Value JsonValue;

		Edge(string _Type, FBNode * _Node, long _RowNumber) :
			Type(_Type),
			Node(_Node),
			RowNumber(_RowNumber)
		{}

		Edge(string _Type, const string & _Value) :
			Type(_Type),
			Value(_Value),
			Node(NULL)
		{}

			
		Edge(string _Type, json_spirit::Value & _JsonValue) :
			Type(_Type),
			JsonValue(_JsonValue),
			Node(NULL)
		{}


		string id() const;
		
		unsigned long long numericId() const;
		facebook_time timestamp() const;
	};
		
	struct EdgeTypeIndex {};
	struct TimeOrderedEdgeType {};
	struct AscTimeOrderedEdgeType {};
	struct EdgeTypeRowNumIndex {};
	
	typedef boost::multi_index_container
		<
			Edge,
			boost::multi_index::indexed_by
			<
				boost::multi_index::ordered_unique
				<
					boost::multi_index::tag<EdgeTypeIndex>,
					boost::multi_index::composite_key
					<
						Edge, 
						boost::multi_index::member<Edge,string,&Edge::Type>,
						boost::multi_index::const_mem_fun<Edge, unsigned long long, &Edge::numericId> 
					>,
					boost::multi_index::composite_key_compare<
						std::less<std::string>,   
						std::less<unsigned long long> 
					>
				>,									
				boost::multi_index::ordered_unique
				<
					boost::multi_index::tag<TimeOrderedEdgeType>,
					boost::multi_index::composite_key
					<
						Edge, 
						boost::multi_index::member<Edge,string,&Edge::Type>,
						boost::multi_index::const_mem_fun<Edge,facebook_time,&Edge::timestamp>,
						boost::multi_index::const_mem_fun<Edge, unsigned long long, &Edge::numericId> 
					>,
					boost::multi_index::composite_key_compare<
						std::less<std::string>,   
						std::greater<facebook_time>,						
						std::less<unsigned long long> 
					>
				>,							
				boost::multi_index::ordered_unique
				<
					boost::multi_index::tag<AscTimeOrderedEdgeType>,
					boost::multi_index::composite_key
					<
						Edge, 
						boost::multi_index::member<Edge,string,&Edge::Type>,
						boost::multi_index::const_mem_fun<Edge,facebook_time,&Edge::timestamp>,
						boost::multi_index::const_mem_fun<Edge, unsigned long long, &Edge::numericId> 
					>,
					boost::multi_index::composite_key_compare<
						std::less<std::string>,   
						std::less<facebook_time>,						
						std::less<unsigned long long> 
					>
				>,						
				boost::multi_index::ordered_unique
				<
					boost::multi_index::tag<EdgeTypeRowNumIndex>,
					boost::multi_index::composite_key
					<
						Edge, 
						boost::multi_index::member<Edge,string,&Edge::Type>,
						boost::multi_index::member<Edge,long,&Edge::RowNumber>
					>,
					boost::multi_index::composite_key_compare<
						std::less<std::string>,   
						std::less<long>
					>
				>
			>
		> EdgeContainer;

	class FBNode : public NodeBase 
	{
		
	private:
		struct EdgeLoadSpec {
			
			long loadedCount;
			long requestedCount;
			bool hasReachedEnd;

			EdgeLoadSpec() :
				loadedCount(0),
				requestedCount(0),
				hasReachedEnd(false)
			{ }
							
			EdgeLoadSpec(long _loadedCount, long _requestedCount) :
				loadedCount(_loadedCount),
				requestedCount(_requestedCount)
			{ }

			EdgeLoadSpec(long _loadedCount, long _requestedCount, bool _hasReachedEnd) :
				loadedCount(_loadedCount),
				requestedCount(_requestedCount), 
				hasReachedEnd(_hasReachedEnd)
			{ }

		};

		boost::function<void()> loadCompleteDelegate;

	public:
		static string tryExtractId(vector<json_spirit::Pair> & obj, bool & success, facebook_time & timestamp);

	public:
		map<string,EdgeLoadSpec> loadState;
		EdgeContainer Edges, ReverseEdges;
		
		FBNode(string id);


		string getId()
		{
			return id;
		}

		unsigned long long getNumericId();
		facebook_time getTimestamp();

		std::string getURI()
		{
			return id;
		}

		bool edgeLimitReached(string edge);

		string getAttribute(string key);

		virtual FBNode * buildChildFromJSON(string edge, vector<json_spirit::Pair> & obj);
		virtual void addJSON(string edge, vector<json_spirit::Pair> & obj);
		void addJSONArray(string edge, vector<json_spirit::Value> & objectArray);

		bool setLoadTarget(string edge, long count);
		void startLoad();

		void clearLoadCompleteDelegate();
		void setLoadCompleteDelegate(boost::function<void()> loadCompleteDelegate);
		
		string getNodeType();
		void setNodeType(string nodeType);

		void setTimestamp(facebook_time timestamp);

	protected:
		string id;
		string nodeType;
		unsigned long long numericId;
		facebook_time timestamp;


	};
}
#endif