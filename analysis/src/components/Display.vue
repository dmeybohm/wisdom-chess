<template>
  <div class="searches">
    <div class="left-column">
      <h2>Starting:</h2>
      <Chessboard v-if="searches.length > 0" :fen="searches[0].fen" :move_list="[]" />

      <h4>Best Moves:</h4>
      <div v-for="search in searches" :key="search.id" >
        <Chessboard :fen="search.fen" :move_list="[search.move]" v-once @click.native="loadPositions(search)" />
        <div>Depth: {{search.depth}}</div>
        <div>Move: {{search.move}}</div>
      </div>
    </div>
    <div class="right-column">
      <MoveStack :selected_search="selected_search" />
    </div>
  </div>
</template>

<script>
import Chessboard from "./Chessboard";
import MoveStack from "./MoveStack";

export default {
  name: "Display",
  components: {MoveStack, Chessboard},
  data: function() {
    return {
      searches: [],
      selected_search: null,
      positions: [],
      rebind_count: 0
    }
  },
  methods: {
    loadPositions: function(search) {
      let url = 'http://localhost:8000/api/fetch.php?object=positions&decision_id='+encodeURIComponent(search.decision_id);
      this.selected_search = search;
      fetch(url)
      .then(data => data.json())
      .then(data => {
        this.positions = data;
        this.rebind_count++;
      })
    }
  },
  mounted: function() {
    fetch('http://localhost:8000/api/fetch.php?object=searches')
        .then(data => data.json())
        .then(data => {
          this.searches = data;
          this.starting_position = data[0];
        })
  }
}
</script>

<style scoped>
.searches {
  display: flex;
  margin-left: .2rem;
}

.left-column {
  width: 30%;
  max-height: 95vh;
  overflow-y: scroll;
}

.right-column {
  width: 70%;
  max-height: 95vh;
  overflow-y: scroll;
}
</style>