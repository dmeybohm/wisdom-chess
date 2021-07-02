<template>
  <div class="searches">
    <div class="column">
      <h1>Iterative Searches</h1>
      <div v-for="iterative_search in iterative_searches" :key="iterative_search.id">
        <Chessboard :fen="iterative_search.fen" :move_list="iterative_search.move_list"/>
        <MoveWithScore v-if="selected_search !== null"
                       :move="selected_search.move"
                       :score="selected_search.score" />
        <ol class="iteration-list">
          <li v-for="(depth, index) in iterative_search.iterations" :key="`iteration-${index}`">
            <button @click="loadSearch(iterative_search, depth)">Depth {{ depth }}</button>
          </li>
        </ol>
        <hr/>
      </div>
    </div>
    <div class="right-column">
      <MoveStack :url="url" :selected_search="selected_search"/>
    </div>
  </div>
</template>

<script>
import Chessboard from "./Chessboard";
import MoveStack from "./MoveStack";
import MoveWithScore from "@/components/MoveWithScore";

export default {
  name: "Display",
  components: {MoveWithScore, MoveStack, Chessboard},
  data: function () {
    return {
      iterative_searches: [],
      selected_search: null,
      current_depth: null,
      searches: [],
      positions: [],
      url: 'http://localhost:8000'
    }
  },
  mounted: function () {
    fetch(this.url + '/api/fetch.php?object=iterative_searches')
        .then(data => data.json())
        .then(data => {
          data.forEach(elem => elem.move_list = [])
          this.iterative_searches = data;
        })
  },
  methods: {
    loadSearch: function (iterative_search, depth) {
      let params = '&iterative_search_id=' + encodeURIComponent(iterative_search.id);
      params += '&depth=' + encodeURIComponent(depth);
      let url = this.url + '/api/fetch.php?object=searches' + params;
      fetch(url)
          .then(data => data.json())
          .then(data => {
            data.forEach(elem => {
              elem.move_list = [ elem.move ].filter(Boolean)
              elem.fen = iterative_search.fen;
              iterative_search.move_list = elem.move_list
            });
            this.selected_search = data[0];
          })
    },

    loadPositions: function (search) {
      let url = this.api + '/api/fetch.php?object=positions&decision_id=' + encodeURIComponent(search.decision_id);
      this.selected_search = search;
      fetch(url)
          .then(data => data.json())
          .then(data => {
            this.positions = data;
          })
    }
  }
}
</script>

<style scoped>
.searches {
  display: flex;
  margin-left: .2rem;
}

.column {
  width: 400px;
  max-height: 95vh;
  overflow-y: auto;
  text-align: center;
  display: flex;
  align-items: center;
  flex-direction: column;
}

.right-column {
  max-height: 95vh;
  overflow-y: auto;
}

.iteration-list {
  list-style-type: none;
  display: flex;
  align-items: center;
  margin: 0;
  padding: 0;
  max-width: 15rem;
}

.iteration-list li {
  margin-right: 5px;
}
</style>