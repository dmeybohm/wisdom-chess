<template>
  <div v-if="selected_search !== null">
    <div v-for="position in positions" :key="position.id">
      <div class="move">
        <Chessboard :fen="selected_search.fen" :move_list="position.move_list" v-once  />
        <div class="rhs">
          <div>{{position.move}}</div>
          <div>{{position.score}}</div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import Chessboard from "./Chessboard";
export default {
  name: "MoveStack",
  components: {Chessboard},
  data: function() {
    return {
      // todo make multidimensional
      positions: []
    }
  },
  props: {
    selected_search: Object,
  },
  watch: {
    selected_search: function(new_val, old_val) {
      console.log(new_val, old_val);
      let decisionId = encodeURIComponent(this.selected_search.decision_id);
      let url = 'http://localhost:8000/api/fetch.php?object=positions&decision_id='+ decisionId;
      fetch(url)
          .then(data => data.json())
          .then(data => {
            data.forEach(position => position.move_list = [position.move]);
            this.positions = data;
            this.rebind_count++;
          })

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
</style>