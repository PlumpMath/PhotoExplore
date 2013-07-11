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


using namespace std;

namespace Facebook {

	class FBNode;

	class FBDataSource {

	public:
		virtual void loadField(FBNode * parent, string nodeQuery, string interpretAs = "") = 0;
		virtual void load(FBNode * parent, string id, string edge) = 0;
		virtual void loadQuery(FBNode * parent, string nodeQuery, string interpretAs) = 0;
		static FBDataSource * instance;


	};


	struct Edge {

		string Type;		
		FBNode * Node;

		string Value;
		json_spirit::Value JsonValue;
		//map<string,string> ObjectValue;
		//vector<map<string,string> > ObjectArray;

		Edge(string _Type, FBNode * _Node) :
			Type(_Type),
			Node(_Node)
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

		//Edge(string _Type, const map<string,string> & _ObjectValue) :
		//	Type(_Type),
		//	Node(NULL),
		//	ObjectValue(_ObjectValue)
		//{}

		//Edge(string _Type, const vector<map<string,string> > & _ObjectArray) :
		//	Type(_Type),
		//	Node(NULL),
		//	ObjectArray(_ObjectArray)
		//{}


		string id() const;
	};
		
	struct EdgeTypeIndex {};
	struct EdgeSequence {};
	
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
						boost::multi_index::const_mem_fun<Edge, string, &Edge::id> 
					>
				>,								
				boost::multi_index::sequenced
				<
					boost::multi_index::tag<EdgeSequence>
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
		static string tryExtractId(vector<json_spirit::Pair> & obj, bool & success);

	public:
		map<string,EdgeLoadSpec> loadState;
		EdgeContainer Edges;
		
		FBNode(string id);


		string getId()
		{
			return id;
		}

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

		virtual void setDataPriority(float priority);

		string getNodeType();
		void setNodeType(string nodeType);

		void update();

		bool loadCompleteFlag;

	protected:
		string id;
		string nodeType;


	};
}
#endif