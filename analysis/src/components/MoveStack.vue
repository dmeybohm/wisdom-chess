<template>
  <div v-if="selected_search !== null">
    <div class="columns">
      <div class="column">
        <PositionList :fen="selected_search.fen" :positions="positions" @selected="select_position(0)"/>
      </div>
      <div class="column" v-for="selected_position in selected_positions" :key="selected_position.id">
        <div for="position in positions" :key="position.id">
          <div class="move">
            <Chessboard :fen="selected_search.fen" :move_list="position.move_list" v-once/>
            <div class="rhs">
              <div>{{ position.move }}</div>
              <div>{{ position.score }}</div>
            </div>
          </div>
        </div>
      </div>
    </div>

  </div>
</template>

<script>
import Chessboard from "./Chessboard";
import PositionList from "./PositionList";

export default {
  name: "MoveStack",
  components: {PositionList, Chessboard},
  data: function() {
    return {
      // todo make multidimensional
      positions: [],
      selected_positions: []
    }
  },
  props: {
    url: String,
    selected_search: Object
  },
  watch: {
    selected_search: function(new_val, old_val) {
      console.log(new_val, old_val);
      let decisionId = encodeURIComponent(this.selected_search.decision_id);
      let url = this.url + '/api/fetch.php?object=positions&decision_id='+ decisionId;
      fetch(url)
          .then(data => data.json())
          .then(data => {
            data.forEach(position => position.move_list = [position.move]);
            this.positions = data;
          })
    }
  },
  methods: {
    select_position: function(position) {
      console.log(position);
    }
  }
}
</script>

<style scoped>
.move {
  display: flex;
}
.rhs {
  text-align: center;
}
.columns {
  display: flex;
  flex-diration: row;
}
</style>